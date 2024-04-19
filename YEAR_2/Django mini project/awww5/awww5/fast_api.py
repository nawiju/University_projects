import os
import sqlite3
import django
from django.conf import settings
os.environ["DJANGO_SETTINGS_MODULE"] = "awww5.settings"

django.setup()

from fastapi import FastAPI, HTTPException
from typing import List
from asgiref.sync import sync_to_async
from tortoise.contrib.fastapi import HTTPNotFoundError
from image_gallery.models import Tag, SVGImage
from pydantic import BaseModel
from fastapi import HTTPException
from fastapi import Request

app = FastAPI()

class TagWithImageCount(BaseModel):
    name: str
    image_count: int

@app.get("/tags", response_model=List[TagWithImageCount])
async def get_tags():
    """
    Retrieve all tags with the count of associated images.
    
    Example request:
        curl -X 'GET'  'http://localhost:8000/tags' -H 'accept: application/json'

    Returns:
        List[TagWithImageCount]: A list of tags with their associated image count.
    """
    return await sync_to_async(get_tags_sync)()

def get_tags_sync():
    tags = Tag.objects.all()
    return [TagWithImageCount(name=tag.name, image_count=tag.images.count()) for tag in tags]

@app.post("/tags_post")
async def get_tags_post():
    """
    Retrieve all tags with the count of associated images.
    
    Example request:
        curl -X 'POST' 'http://0.0.0.0:8000/tags_post'   -H 'accept: application/json'   -H 'Content-Type: application/x-www-form-urlencoded' 

    Returns:
        List[TagWithImageCount]: A list of tags with their associated image count.
    """
    return await sync_to_async(get_tags_sync)()


@app.get("/images")
async def get_images():
    """
    Retrieve all images with their associated tags.
    
    Example request:
        curl -X 'GET'  'http://0.0.0.0:8000/images' -H 'accept: application/json'

    Returns:
        List[dict]: A list of images with their associated tags.
    """
    return await sync_to_async(get_images_sync)()

def get_images_sync():
    images = SVGImage.objects.all()
    
    def image_with_tags(image):
        tags = image.tags.all()
        return {
            "id": image.id,
            "title": image.title,
            "tags": [tag.name for tag in tags]
        }
        
    return [image_with_tags(image) for image in images]

@app.post("/images_post")
async def get_images_post():
    """
    Retrieve all images with their associated tags.

    Example request:
        curl -X 'POST'  'http://0.0.0.0:8000/images_post' -H 'accept: application/json'

    Returns:
        List[dict]: A list of images with their associated tags.
    """
    return await sync_to_async(get_images_sync)()


@app.get("/images/{tag}")
async def get_images_by_tag(tag: str):
    """
    Retrieve all images with a specific tag.

    Args:
        tag (str): The tag to filter images by.
        
    Example request:
        curl -X 'GET'  'http://0.0.0.0:8000/images/listerine' -H 'accept: application/json'


    Returns:
        List[dict]: A list of images with the specified tag.
    """
    return await sync_to_async(get_images_by_tag_sync)(tag)

def get_images_by_tag_sync(tag):
    tag = Tag.objects.filter(name=tag).first()
    if tag is None:
        raise HTTPException(status_code=404, detail="Tag not found")
    
    images = tag.images.all()
    
    return [{"title": image.title, "id": image.id} for image in images]

@app.post("/images_post/{tag}")
async def get_images_by_tag_post(tag: str):
    """
    Retrieve all images with a specific tag.

    Args:
        tag (str): The tag to filter images by.
        
    Example request body:
    

    Returns:
        List[dict]: A list of images with the specified tag.
    """
    return await sync_to_async(get_images_by_tag_sync)(tag)


@app.post("/images/del")
async def delete_images(request: Request):
    """
    Delete images with the specified IDs.

    Args:
        request (Request): The HTTP request containing the IDs of the images to delete.
        
    Example request body:
        curl -X POST "http://0.0.0.0:8000/images/del" -H "Content-Type: application/json" -d '{"image_ids": [62, 63]}'

    Returns:
        None
    """
    ids = await request.json()
    await sync_to_async(delete_images_sync)(ids["image_ids"])
    return
    
def delete_images_sync(ids):
    for id in ids:
        image = SVGImage.objects.filter(id=id).first()
        if image is not None:
            image.delete()
            
            
BASEDIR = os.path.dirname(os.path.abspath(__file__))
STATIC_DIR = os.path.join(BASEDIR, 'db.sqlite3')
            
@app.get("/tags_2")
async def get_tags_2():
    """
    Retrieves all tags from the image_gallery_tag table along with the count of images associated with each tag.
    
    Returns:
        A list of tuples, where each tuple contains the tag ID and a TagWithImageCount object.
    """
    
    conn = sqlite3.connect(STATIC_DIR)
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM image_gallery_tag")
    rows = cursor.fetchall()
    result = []
    
    for row in rows:
        count_query = f"SELECT COUNT(*) FROM image_gallery_tag_images WHERE tag_id = {row[0]}"
        cursor.execute(count_query)
        count = cursor.fetchone()[0]
        print(count)
        result.append([row[0], TagWithImageCount(name=row[1], image_count=count)])
        
    conn.close()
    return result


@app.get("/images_2")
async def get_images_2():
    """
    Retrieves images and their associated tags from the database.

    Returns:
        list: A list of image data, where each element is a list containing the image ID, image URL, and a list of tag names.
    """
    
    conn = sqlite3.connect(STATIC_DIR)
    cursor = conn.cursor()
    
    cursor.execute("SELECT * FROM image_gallery_svgimage")
    rows = cursor.fetchall()
    
    cursor.execute("SELECT * FROM image_gallery_tag_images")
    tags = cursor.fetchall() 
    
    result = []
    
    for row in rows:        
        tag_ids_query = f"SELECT tag_id FROM image_gallery_tag_images WHERE svgimage_id = {row[0]}"
        cursor.execute(tag_ids_query)
        tag_ids = cursor.fetchall()

        tag_names_query = f"SELECT name FROM image_gallery_tag WHERE id IN ({','.join(str(tag_id[0]) for tag_id in tag_ids)})"
        cursor.execute(tag_names_query)
        tag_names = cursor.fetchall()

        result.append([row[0], row[1], [tag_name[0] for tag_name in tag_names]])

    return result


@app.get("/images_2/{tag}")
async def get_images_by_tag_post_2(tag: str):
    """
    Retrieves images by tag from the image gallery.

    Args:
        tag (str): The tag to filter the images by.
        
    Example:
        curl -X 'GET'  'http://localhost:8000/images_2/listerine' -H 'accept: application/json'

    Returns:
        dict: A dictionary containing the tag, images, and image IDs.

    Example:
        >>> get_images_by_tag_post("nature")
        {
            "tag": "nature",
            "images": ["image1.jpg", "image2.jpg"],
            "ids": [1, 2]
        }
    """
    
    conn = sqlite3.connect(STATIC_DIR)
    cursor = conn.cursor()
    
    tag_id_query = f"SELECT id FROM image_gallery_tag WHERE name = '{tag}'"
    cursor.execute(tag_id_query)
    tag_id = cursor.fetchone()[0]

    image_names_query = f"SELECT svgimage.title, svgimage.id FROM image_gallery_svgimage AS svgimage JOIN image_gallery_tag_images AS tag_images ON svgimage.id = tag_images.svgimage_id WHERE tag_images.tag_id = {tag_id}"
    cursor.execute(image_names_query)
    image_names = cursor.fetchall()

    images =  [image_name[0] for image_name in image_names]
    images_id = [image_name[1] for image_name in image_names]
    
    return {"tag": tag, "images": images, "ids": images_id}


@app.post("/images_2/del")
async def delete_images_2(request: Request):
    """
    Delete images from the image gallery based on the provided image IDs.

    Args:
        request (Request): The incoming request object.
        
    Example:
        curl -X POST "http://0.0.0.0:8000/images_2/del" -H "Content-Type: application/json" -d '{"image_ids": [60]}'

    Returns:
        None
    """
    
    conn = sqlite3.connect(STATIC_DIR)
    cursor = conn.cursor()
    
    
    ids = await request.json()
    for id in ids["image_ids"]:
        cursor.execute(f"DELETE FROM image_gallery_svgimage WHERE id = {id}")
    conn.commit()
    conn.close()
    
    return  