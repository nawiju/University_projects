from django.shortcuts import redirect, render
from .models import SVGImage, Rectangle, Tag, User
from django.core.paginator import Paginator

MAX_PAGE_SIZE = 12

def index(request):
    images = SVGImage.objects.all()
    images_rectangles = [(image, image.rectangles.all()) for image in images]
    
    paginator = Paginator(images_rectangles, MAX_PAGE_SIZE)  

    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    
    if request.method == 'POST':
        if 'searched_tag' in request.POST:
            tag_name = request.POST.get('tag_name')
            
            if tag_name == '':
                return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj})
            
            tag = Tag.objects.filter(name=tag_name).first()
            
            if tag is None:
                images = []
                images_rectangles = []
                paginator = Paginator(images_rectangles, MAX_PAGE_SIZE)
                page_obj = paginator.get_page(page_number)
                return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj})
            
            images = tag.images.all()
            images_rectangles = [(image, image.rectangles.all()) for image in images]
            paginator = Paginator(images_rectangles, MAX_PAGE_SIZE)  

            page_number = request.GET.get('page')
            page_obj = paginator.get_page(page_number)
            return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj})
        
        if 'filter_button' in request.POST:
            filter_by = request.POST.get('filter')
            if filter_by == 'otn':
                images = SVGImage.objects.order_by('publication_date')
                images_rectangles = [(image, image.rectangles.all()) for image in images]
                
                paginator = Paginator(images_rectangles, MAX_PAGE_SIZE)  

                page_number = request.GET.get('page')
                page_obj = paginator.get_page(page_number)
                return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj})
           
            if filter_by == 'nto':
                images = SVGImage.objects.order_by('-publication_date')
                images_rectangles = [(image, image.rectangles.all()) for image in images]
                
                paginator = Paginator(images_rectangles, MAX_PAGE_SIZE)  

                page_number = request.GET.get('page')
                page_obj = paginator.get_page(page_number)
                return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj})
    
    return render(request, 'image_gallery/image_list.html', {'page_obj': page_obj}) 

def view_svg(request, image_id):
    image = SVGImage.objects.get(id=image_id)
    rectangles = image.rectangles.all()
    tags = image.tags.all()
    editors = image.editors.all()
    
    is_admin = request.user.is_superuser
    
    is_staff = False
    if request.user in editors and request.user.is_staff:
        is_staff = True
    elif is_admin:
        is_staff = True
    else:
        is_staff = False
    
    if request.method == 'POST' and is_staff:
        if 'delete_rectangle' in request.POST:
            rectangle_id = request.POST.get('delete_rectangle')
            Rectangle.objects.filter(pk=rectangle_id).delete()
            return redirect('view_svg', image_id=image_id)
        
        if 'update_description' in request.POST:
            description = request.POST.get('description')
            image.description = description
            image.save()
            return redirect('view_svg', image_id=image_id)
        
        if 'add_tag' in request.POST:
            tag_name = request.POST.get('tag_name')
            try:
                tag, created = Tag.objects.get_or_create(name=tag_name)
            except Tag.MultipleObjectsReturned:
                tag = Tag.objects.filter(name=tag_name).first()
            tag.images.add(image)
            tag.save()
            image.save()
            return redirect('view_svg', image_id=image_id)
        
        if 'remove_editor' in request.POST:
            editor_id = request.POST.get('remove_editor')
            editor = User.objects.get(pk=editor_id)
            image.remove_editor(editor)
            return redirect('view_svg', image_id=image_id)
        
        x = request.POST.get('x')
        y = request.POST.get('y')
        width = request.POST.get('width')
        height = request.POST.get('height')
        color = request.POST.get('color')

        Rectangle.objects.create(image=image, x=x, y=y, width=width, height=height, color=color)
        image.save()

        return redirect('view_svg', image_id=image_id)
    
    return render(request, 'image_gallery/view_svg.html', {'image': image, 'rectangles': rectangles, 'is_staff': is_staff, 'tags': tags, 'is_admin': is_admin, 'editors': editors})
