# Generated by Django 5.0.3 on 2024-03-30 20:52

import django.db.models.deletion
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('image_gallery', '0003_svgimage_rectangles'),
    ]

    operations = [
        migrations.RenameField(
            model_name='svgimage',
            old_name='rectangles',
            new_name='svg_code',
        ),
        migrations.CreateModel(
            name='Rectangle',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('x', models.IntegerField()),
                ('y', models.IntegerField()),
                ('width', models.IntegerField()),
                ('height', models.IntegerField()),
                ('color', models.CharField(max_length=20)),
                ('svg_image', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='image_gallery.svgimage')),
            ],
        ),
    ]