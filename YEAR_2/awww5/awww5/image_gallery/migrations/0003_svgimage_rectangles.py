# Generated by Django 5.0.3 on 2024-03-30 19:37

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('image_gallery', '0002_remove_svgimage_svg_code_svgimage_height_and_more'),
    ]

    operations = [
        migrations.AddField(
            model_name='svgimage',
            name='rectangles',
            field=models.TextField(default=''),
        ),
    ]