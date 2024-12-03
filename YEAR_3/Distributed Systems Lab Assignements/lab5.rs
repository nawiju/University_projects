use std::io::{Read, Write};
use rustls::{pki_types::{pem::PemObject, ServerName}, RootCertStore, ClientConnection, ServerConnection};
use hmac::{Hmac, Mac};
use sha2::Sha256;
use std::sync::Arc;

type HmacSha256 = Hmac<Sha256>;
// You can add here other imports from std or crates listed in Cargo.toml.

// The below `PhantomData` marker is here only to suppress the "unused type
// parameter" error. Remove it when you implement your solution:
use std::marker::PhantomData;

pub struct SecureClient<L: Read + Write> {
    // Add here any fields you need.
    phantom: PhantomData<L>,
    hmac_key: Vec<u8>,
    link: L,
    connection: ClientConnection,
}

pub struct SecureServer<L: Read + Write> {
    // Add here any fields you need.
    phantom: PhantomData<L>,
    link: L,
    hmac_key: Vec<u8>,
    connection: ServerConnection,
}

impl<L: Read + Write> SecureClient<L> {
    /// Creates a new instance of SecureClient.
    ///
    /// SecureClient communicates with SecureServer via `link`.
    /// The messages include a HMAC tag calculated using `hmac_key`.
    /// A certificate of SecureServer is signed by `root_cert`.
    /// We are connecting with `server_hostname`.
    pub fn new(
        link: L,
        hmac_key: &[u8],
        root_cert: &str,
        server_hostname: ServerName<'static>,
    ) -> Self {
        let mut root_store = RootCertStore::empty();
        
        root_store.add_parsable_certificates(rustls::pki_types::CertificateDer::from_pem_slice(
            root_cert.as_bytes(),
        ));

        let client_config = rustls::ClientConfig::builder()
            .with_root_certificates(root_store)
            .with_no_client_auth();

        let connection =
            ClientConnection::new(Arc::new(client_config), server_hostname.clone().try_into().unwrap()).unwrap();

        SecureClient {
            phantom: PhantomData,
            hmac_key: hmac_key.to_vec(),
            link,
            connection,
        }
    }

    /// Sends the data to the server. The sent message follows the
    /// format specified in the description of the assignment.
    pub fn send_msg(&mut self, data: Vec<u8>) {
        // 4 bytes: length (encoded in network byte order) of the message’s content (i.e., the size of data passed to the send_msg() method of SecureClient),
        // content of the message (i.e., the data passed to the send_msg() method),
        // 32 bytes: a HMAC tag (SHA256 based) of the message’s content (i.e., of the data passed to the send_msg() method).

        let message_length = data.len() as u32;
        let mut mac = HmacSha256::new_from_slice(&self.hmac_key).unwrap();
        mac.update(&data);
        let tag = mac.finalize().into_bytes();

        let mut message = vec![];
        message.extend_from_slice(&message_length.to_be_bytes());
        message.extend_from_slice(&data);
        message.extend_from_slice(&tag);

        // The messages shall be encrypted with TLS and sent over the link provided as an argument to the new() method of SecureClient.
        let mut tls_stream = rustls::Stream::new(&mut self.connection, &mut self.link);
        tls_stream.write_all(&message).unwrap();
        tls_stream.flush().unwrap();
    }
}

impl<L: Read + Write> SecureServer<L> {
    /// Creates a new instance of SecureServer.
    ///
    /// SecureServer receives messages from SecureClients via `link`.
    /// HMAC tags of the messages are verified against `hmac_key`.
    /// The private key of the SecureServer's certificate is `server_private_key`,
    /// and the full certificate chain is `server_full_chain`.
    pub fn new(
        link: L,
        hmac_key: &[u8],
        server_private_key: &str,
        server_full_chain: &str,
    ) -> Self {
        let certs =
            rustls::pki_types::CertificateDer::pem_slice_iter(server_full_chain.as_bytes())
                .flatten()
                .collect();

        let private_key =
            rustls::pki_types::PrivateKeyDer::from_pem_slice(server_private_key.as_bytes())
                .unwrap();

        let server_config = rustls::ServerConfig::builder()
            .with_no_client_auth()
            .with_single_cert(certs, private_key)
            .unwrap();

        let connection = ServerConnection::new(Arc::new(server_config)).unwrap();

        SecureServer {
            phantom: PhantomData,
            link,
            hmac_key: hmac_key.to_vec(),
            connection,
        }
    }

    /// Receives the next incoming message and returns the message's content
    /// (i.e., without the message size and without the HMAC tag) if the
    /// message's HMAC tag is correct. Otherwise, returns `SecureServerError`.
    pub fn recv_message(&mut self) -> Result<Vec<u8>, SecureServerError> {
        // SecureServer decrypts messages and validates their HMAC tags. 
        // If the tag is invalid, an error is returned.
        // Otherwise, the server returns the content of the message (i.e., without the length and the HMAC tag).
        let mut tls_stream = rustls::Stream::new(&mut self.connection, &mut self.link);

        // Read the length of the message
        let mut length_bytes = [0u8; 4];
        tls_stream.read_exact(&mut length_bytes).unwrap();
        let message_length = u32::from_be_bytes(length_bytes) as usize;

        // Read the message content
        let mut message_content = vec![0u8; message_length];
        tls_stream.read_exact(&mut message_content).unwrap();

        // Read the HMAC tag
        let mut tag = vec![0u8; 32];
        tls_stream.read_exact(&mut tag).unwrap();

        // Verify the HMAC tag
        let mut mac = HmacSha256::new_from_slice(&self.hmac_key).unwrap();
        mac.update(&message_content);
        match mac.verify_slice(&tag) {
            Ok(_) => Ok(message_content),
            Err(_) => Err(SecureServerError::InvalidHmac),
        }
    }
}

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
pub enum SecureServerError {
    /// The HMAC tag of a message is invalid.
    InvalidHmac,
}

// You can add any private types, structs, consts, functions, methods, etc., you need.
