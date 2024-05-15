import random
import asyncio
from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel
import sqlite3
from typing import List
from fastapi import FastAPI, BackgroundTasks

from fastapi import FastAPI
from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware

import websockets

app = FastAPI()

# CORS configuration
origins = [
    "http://localhost",
    "http://localhost:3000", 
    "http://localhost:8000", 
    "http://0.0.0.0:8000", 
    "http://0.0.0.0:8001",
    "http://0.0.0.0:8002",
    "http://0.0.0.0:8003",
    "http://localhost:8002",
    "http://localhost:8003",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE"],
    allow_headers=["*"],
)

def get_db():
    db = sqlite3.connect('db.db')
    try:
        yield db
    finally:
        db.close()
        
class Image(BaseModel):
    title: str
    width: int
    height: int
    rectangles: list

class Rectangle(BaseModel):
    x1: int
    y1: int
    x2: int
    y2: int
    
def get_last_5_entries() -> List[dict]:
    conn = sqlite3.connect('db.db')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM images ORDER BY id DESC LIMIT 5")
    entries = cursor.fetchall()
    
    images = []
    
    for entry in entries:
        cursor.execute("SELECT * FROM rectangles WHERE image_id=?", (entry[0],))
        rectangles = cursor.fetchall()
        images.append({"image": entry, "rectangles": rectangles})
    
    conn.close()
    return images
    
@app.websocket("/subscribe-latest")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    
    last_entries = get_last_5_entries()
    first  = True
    
    while True:
        current_entries = get_last_5_entries()
        
        if (first):
            await websocket.send_json(current_entries)
            first = False
            
        if current_entries != last_entries and not first:
            await websocket.send_json(current_entries)

            last_entries = current_entries
            
        await asyncio.sleep(5)
    
@app.post("/rectangle_delete/")
async def delete_rectangle(data: dict):
    conn = sqlite3.connect('db.db')
    cursor = conn.cursor()
    
    cursor.execute("SELECT * FROM rectangles WHERE image_id=? AND x1=? AND y1=? AND x2=? AND y2=?", (data['image_id'], data['x_1'], data['y_1'], data['x_2'], data['y_2']))
    rectangle = cursor.fetchone()
    
    if not rectangle:
        raise HTTPException(status_code=404, detail="Rectangle not found")
    
    cursor.execute("DELETE FROM rectangles WHERE image_id=? AND x1=? AND y1=? AND x2=? AND y2=?", (data['image_id'], data['x_1'], data['y_1'], data['x_2'], data['y_2']))
    
    conn.commit()
    conn.close()
    
    return JSONResponse(content={"message": "Rectangle deleted successfully"})
    
@app.post("/rectangle_add/")
async def create_rectangle(data: dict):
    image_id = data['image_id']
    conn = sqlite3.connect('db.db')
    cursor = conn.cursor()
    
    cursor.execute("SELECT * FROM images WHERE id=?", (image_id,))
    image = cursor.fetchone()
    
    if not image:
        raise HTTPException(status_code=404, detail="Image not found")
    
    cursor.execute("INSERT INTO rectangles (image_id, x1, y1, x2, y2, color) VALUES (?, ?, ?, ?, ?, ?)", (image_id, data['x_1'], data['y_1'], data['x_2'], data['y_2'], data['color']) )
    
    conn.commit()
    conn.close()
    
    return JSONResponse(content={"message": "Rectangle created successfully"})
    
    
@app.post("/images/")
async def create_image(image: Image):
    conn = sqlite3.connect('db.db')
    cursor = conn.cursor()
    cursor.execute("INSERT INTO images (title, width, height) VALUES (?, ?, ?)", (image.title, image.width, image.height))
    
    image_id = cursor.lastrowid
    
    for rectangle in image.rectangles:
        cursor.execute("INSERT INTO rectangles (image_id, x1, y1, x2, y2, color) VALUES (?, ?, ?, ?, ?, ?)", (image_id, rectangle['x_1'], rectangle['y_1'], rectangle['x_2'], rectangle['y_2'], rectangle['color']) )
    
    conn.commit()
    conn.close()
    
    return JSONResponse(content={"message": "Image created successfully"})

async def generate_image():
    image = {
        0: 1,
        1: "Image 1",
        2: random.randint(100, 1000),
        3: random.randint(100, 1000)
    }
    
    rects = []
    
    for i in range(random.randint(10000, 100000)):
        x1 = random.randint(0, image[2])
        y1 = random.randint(0, image[3])
        x2 = random.randint(x1, image[2])
        y2 = random.randint(y1, image[3])
        rect = [0, 0, x1, y1, x2, y2, "#%06x" % random.randint(0, 0xFFFFFF)]
        
        rects.append(rect)
    
    rectangles = rects
    
    return {"image": image, "rectangles": rectangles}

@app.get("/images/{image_id}")
async def get_images(image_id: int, background_tasks: BackgroundTasks):   
    random_number = random.randint(1, 69)
    
    if (random_number < 15):
        await asyncio.sleep(10)
    
    if random_number > 15 and random_number < 25:
        raise HTTPException(status_code=500, detail="Internal Server Error Uga Buga")
    
    if random_number == 26:
        background_tasks.add_task(generate_image)
        weird = await generate_image()
        return JSONResponse(content=weird)
        
    conn = sqlite3.connect('db.db')
    cursor = conn.cursor()
    
    cursor.execute("SELECT * FROM images WHERE id=?", (image_id,))
    image = cursor.fetchone()
    
    if not image:
        raise HTTPException(status_code=404, detail="Image not found")
    
    cursor.execute("SELECT * FROM rectangles WHERE image_id=?", (image_id,))
    rectangles = cursor.fetchall()
        
    conn.close()
    
    return JSONResponse(content={"image": image, "rectangles": rectangles})




# uvicorn fast_api:app --host 0.0.0.0 --port 8001 --reload