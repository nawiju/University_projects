# Generated by Django 5.0.3 on 2024-04-02 13:16

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('image_gallery', '0013_remove_tag_images_tag_images'),
    ]

    operations = [
        migrations.AlterField(
            model_name='tag',
            name='images',
            field=models.ManyToManyField(blank=True, related_name='tags', to='image_gallery.svgimage'),
        ),
    ]
