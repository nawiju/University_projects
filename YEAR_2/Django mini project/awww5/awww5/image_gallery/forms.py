from django import forms
from .models import Rectangle

class RectangleForm(forms.ModelForm):
    class Meta:
        model = Rectangle
        fields = ['x', 'y', 'width', 'height', 'color']
