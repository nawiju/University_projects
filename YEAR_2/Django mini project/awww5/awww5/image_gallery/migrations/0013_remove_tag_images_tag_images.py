# Generated by Django 5.0.3 on 2024-04-02 11:32

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('image_gallery', '0012_remove_tag_images_tag_images'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='tag',
            name='images',
        ),
        migrations.AddField(
            model_name='tag',
            name='images',
            field=models.ManyToManyField(blank=True, to='image_gallery.svgimage'),
        ),
    ]