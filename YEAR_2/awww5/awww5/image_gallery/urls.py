from django.urls import path

from . import views

urlpatterns = [
    path("", views.index, name="index"),
    path('<int:image_id>/', views.view_svg, name='view_svg'),
]

