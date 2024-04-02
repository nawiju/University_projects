import random
import string
from django.core.management.base import BaseCommand
from image_gallery.models import SVGImage, Rectangle

class Command(BaseCommand):
    help = 'Generate random SVG images and add them to the database'

    def add_arguments(self, parser):
        parser.add_argument('count', type=int, help='Number of images to generate')

    def handle(self, *args, **kwargs):
        count = kwargs['count']

        for _ in range(count):
            title = ''.join(random.choices(string.ascii_letters + string.digits, k=10))
            width = random.randint(50, 500)
            height = random.randint(50, 500)

            svg_image = SVGImage.objects.create(
                title=title,
                width=width,
                height=height,
                description=''
            )
            
            for _ in range(random.randint(1, 100)):
                Rectangle.objects.create(
                    image=svg_image,
                    x=random.randint(0, width),
                    y=random.randint(0, height),
                    width=random.randint(5, width),
                    height=random.randint(5, height),
                    color=f'#{random.randint(0, 0xFFFFFF):06x}'
                )

            self.stdout.write(self.style.SUCCESS(f'Successfully created SVG image: {svg_image}'))
