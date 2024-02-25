import setuptools

with open("README.md", "r", encoding="UTF-8") as fh:
    description_2 = fh.read()

setuptools.setup(
    name="bus_project_NJ",
        version="0.0.3",
        author="Natalia J.",
        author_email="njunkiert@gmail.com",
        description="A package for analysing public transport data",
        long_description=description_2,
        long_description_content_type="text/markdown",
        url='https://gitlab.mimuw.edu.pl/nj448267/python_project_nj',
        packages=setuptools.find_packages(),
        package_data={'bus_project_NJ': ['/src/*geojson']},
        classifiers=[
            "Programming Language :: Python :: 3", 
            "License :: OSI Approved :: MIT License",
            "Operating System :: OS Independent"
        ],
        python_requires='>=3.6'
)