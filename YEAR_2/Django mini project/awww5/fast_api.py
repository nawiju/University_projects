import os
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
        url -X 'GET'  'http://0.0.0.0:8000/images' -H 'accept: application/json'

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
        'url -X 'GET'  'http://0.0.0.0:8000/images/listerine' -H 'accept: application/json'


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