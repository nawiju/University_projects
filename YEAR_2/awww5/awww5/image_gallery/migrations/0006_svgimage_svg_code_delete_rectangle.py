# Generated by Django 5.0.3 on 2024-03-30 21:16

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('image_gallery', '0005_remove_svgimage_svg_code'),
    ]

    operations = [
        migrations.AddField(
            model_name='svgimage',
            name='svg_code',
            field=models.TextField(default=''),
        ),
        migrations.DeleteModel(
            name='Rectangle',
        ),
    ]