use std::collections::HashMap;
use std::path::{Path, PathBuf};
use sha2::{Digest, Sha256};
use std::sync::Arc;
use tokio::sync::Mutex;
use tokio::fs::{self as async_fs, File};
use tokio::io::{AsyncReadExt, AsyncWriteExt};

// You can add here other imports from std or crates listed in Cargo.toml.

// You can add any private types, structs, consts, functions, methods, etc., you need.
// As always, you should not modify the public interfaces.

#[async_trait::async_trait]
pub trait StableStorage: Send + Sync {
    /// Stores `value` under `key`.
    async fn put(&mut self, key: &str, value: &[u8]) -> Result<(), String>;

    /// Retrieves value stored under `key`.
    async fn get(&self, key: &str) -> Option<Vec<u8>>;

    /// Removes `key` and the value stored under it.
    async fn remove(&mut self, key: &str) -> bool;
}

struct Storage {
    root_dir: PathBuf,
    key_map: Arc<Mutex<HashMap<String, PathBuf>>>,
}

impl Storage {
    async fn new(root_dir: PathBuf) -> Self {
        async_fs::create_dir_all(&root_dir).await.unwrap();

        let key_map = HashMap::new();

        Storage { root_dir, key_map: Arc::new(Mutex::new(key_map)) }
    }

    fn validate_key(key: &str) -> Result<(), String> {
        if key.is_empty() {
            return Err("Key cannot be empty".to_string());
        } else if key.len() > 255 {
            return Err("Key length exceeds 255 bytes".to_string());
        }
        Ok(())   
    }

    fn validate_value(value: &[u8]) -> Result<(), String> {
        if value.len() > 65535 {
            return Err("Value length exceeds 65535 bytes".to_string());
        }
        Ok(())
    }

    async fn write_tmpfile(&self, value: &[u8], file_path: &Path) -> Result<(), String> {
        let tmpfile_path = self.root_dir.join(file_path.with_extension("tmp"));
        let mut tmpfile = File::create(&tmpfile_path).await.map_err(|e| e.to_string())?;
        let checksum = Sha256::digest(value);

        tmpfile.write_all(&checksum).await.map_err(|e| e.to_string())?;
        tmpfile.write_all(value).await.map_err(|e| e.to_string())?;

        // Call the POSIX fsyncdata function on dstdir/tmpfile to ensure 
        // the data is actually transferred to a disk device
        tmpfile.sync_data().await.map_err(|e| e.to_string())?;

        Ok(())
    }

    async fn write_final_file(&self, file_path: &Path, value: &[u8]) -> Result<(), String> {
        let final_file_path = self.root_dir.join(file_path);
        let mut final_file = File::create(&final_file_path).await.map_err(|e| e.to_string())?;

        final_file.write_all(value).await.map_err(|e| e.to_string())?;

        // Call fsyncdata on dstdir/dstfile.
        final_file.sync_data().await.map_err(|e| e.to_string())?;

        Ok(())
    }

    async fn sync_directory(&self) -> Result<(), String> {
        let dir_file = File::open(&self.root_dir).await.map_err(|e| e.to_string())?;
        dir_file.sync_data().await.map_err(|e| e.to_string())?;

        Ok(())
    }

}

#[async_trait::async_trait]
impl StableStorage for Storage {
    async fn put(&mut self, key: &str, value: &[u8]) -> Result<(), String> {
        Self::validate_key(key)?;
        Self::validate_value(value)?;

        let mut file_path = self.root_dir.join(key);

        // Write the data with a checksum (e.g., CRC32) to a temporary 
        // file dstdir/tmpfile.
        self.write_tmpfile(value, &file_path).await?;

        // Call fsyncdata on dstdir to transfer the data of the modified 
        // directory to the disk device.
        self.sync_directory().await?;

        // Write the data (without the checksum) to dstdir/dstfile.
        self.write_final_file(&file_path, value).await?;

        // Call fsyncdata on dstdir
        if !self.key_map.lock().await.contains_key(key) {
            self.sync_directory().await?;
            self.key_map.lock().await.insert(key.to_string(), file_path);
        }

        // Remove dstdir/tmpfile.
        file_path = self.root_dir.join(key);
        let tmpfile_path = self.root_dir.join(&file_path.with_extension("tmp"));
        async_fs::remove_file(&tmpfile_path).await.unwrap();

        // Call fsyncdata on dstdir
        self.sync_directory().await?;

        Ok(())
    }

    async fn get(&self, key: &str) -> Option<Vec<u8>> {
        let file_path = self.root_dir.join(key);
        let tmpfile_path = file_path.with_extension("tmp");

        if async_fs::metadata(&tmpfile_path).await.is_err() {
            // The tmpfile does not exist. We can return the data from the dstfile.
            if async_fs::metadata(&file_path).await.is_ok() {
                self.key_map.lock().await.insert(key.to_string(), file_path.clone());
            }

            let file_path = {
                let key_map = self.key_map.lock().await;
                key_map.get(key).cloned()
            };

            if let Some(file_path) = file_path {
                let mut file = match File::open(file_path).await {
                    Ok(file) => file,
                    Err(_) => return None,
                };

                let mut buffer = Vec::new();
                match file.read_to_end(&mut buffer).await {
                    Ok(_) => return Some(buffer),
                    Err(_) => return None,
                }
            } else {
                return None;
            }
        }

        // The tmpfile exists. We need to check the checksum.
        let mut tmpfile = match File::open(&tmpfile_path).await {
            Ok(file) => file,
            Err(_) => {
                async_fs::remove_file(&tmpfile_path).await.unwrap_err();
                return None;
            }
        };

        // Verify the checksum.
        let mut checksum = [0; 32];
        if tmpfile.read_exact(&mut checksum).await.is_err() {
            async_fs::remove_file(&tmpfile_path).await.unwrap_err();
            return None; // Could not read the checksum.
        }

        let mut tmpfile_data = Vec::new();
        if tmpfile.read_to_end(&mut tmpfile_data).await.is_err() {
            async_fs::remove_file(&tmpfile_path).await.unwrap_err();
            return None; // Could not read the data.
        }

        // Calculate the checksum of the value.
        let tmpfile_checksum = Sha256::digest(&tmpfile_data);

        if tmpfile_checksum.as_slice() != checksum {
            async_fs::remove_file(&tmpfile_path).await.unwrap_err();
            return None; // Checksums do not match.
        }

        let dstfile_path = self.root_dir.join(key);
        if self.write_final_file(&dstfile_path, &tmpfile_data).await.is_err() {
            return None;
        }

        // Remove the tmpfile after recovery
        if async_fs::remove_file(&tmpfile_path).await.is_err() {
            return None;
        }

        Some(tmpfile_data)
    }

    async fn remove(&mut self, key: &str) -> bool {
        let file_path = {
            let mut key_map = self.key_map.lock().await;
            key_map.remove(key)
        };

        if let Some(file_path) = file_path {
            if async_fs::remove_file(file_path).await.is_ok() {
                self.sync_directory().await.ok();
                let mut key_map = self.key_map.lock().await;
                key_map.remove(key);
                return true;
            }
        }

        false
    }
}


/// Creates a new instance of stable storage.
pub async fn build_stable_storage(root_storage_dir: PathBuf) -> Box<dyn StableStorage> {
    let mut storage = Storage::new(root_storage_dir).await;

    // Rust told me to use the remove method so I did, but it will not be called 
    // because the condition is always false.
    if 1 < 0 {
        let _ = storage.remove("example_key").await;
    }

    Box::new(storage)
}
