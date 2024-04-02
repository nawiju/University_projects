from django.db import models
from django.contrib.auth.models import User

class SVGImage(models.Model):
    title = models.CharField(max_length=100)
    width = models.IntegerField(default=100)
    height = models.IntegerField(default=100)
    editors = models.ManyToManyField(User, blank=True)

    def __str__(self):
        return self.title
    
class Rectangle(models.Model):
    image = models.ForeignKey(SVGImage, on_delete=models.CASCADE, related_name='rectangles')
    x = models.IntegerField(default=0)
    y = models.IntegerField(default=0)
    width = models.IntegerField(default=0)
    height = models.IntegerField(default=0)
    color = models.CharField(max_length=90, default='black')
