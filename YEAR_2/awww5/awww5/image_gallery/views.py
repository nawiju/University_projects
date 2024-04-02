from django.shortcuts import redirect, render
from .models import SVGImage, Rectangle
from .forms import RectangleForm

def index(request):
    images = SVGImage.objects.all()
    return render(request, 'image_gallery/image_list.html', {'images': images})

def view_svg(request, image_id):
    image = SVGImage.objects.get(id=image_id)
    rectangles = image.rectangles.all()
    is_staff = request.user.is_staff
    
    if request.method == 'POST' and is_staff:
        if 'delete_rectangle' in request.POST:
            rectangle_id = request.POST.get('delete_rectangle')
            Rectangle.objects.filter(pk=rectangle_id).delete()
            return redirect('view_svg', image_id=image_id)
        
        x = request.POST.get('x')
        y = request.POST.get('y')
        width = request.POST.get('width')
        height = request.POST.get('height')
        color = request.POST.get('color')

        Rectangle.objects.create(image=image, x=x, y=y, width=width, height=height, color=color)
        image.save()

        return redirect('view_svg', image_id=image_id)
    
    return render(request, 'image_gallery/view_svg.html', {'image': image, 'rectangles': rectangles, 'is_staff': is_staff})
