
interface Rectangle {
    x_1: number;
    y_1: number;
    x_2: number;
    y_2: number;
    color: string;
}

interface Image {
    width: number;
    height: number;
    rectangles: Rectangle[];
}

let to_delete: number = -1;

const exampleImage: Image = {
    width: 800,
    height: 600,
    rectangles: [
        {
            x_1: 100,
            y_1: 100,
            x_2: 200,
            y_2: 200,
            color: 'red',
        },
        {
            x_1: 300,
            y_1: 300,
            x_2: 400,
            y_2: 400,
            color: 'blue',
        },
    ],
};

function latestImages() {
    const imageList = document.getElementById('gallery2');

    const ws = new WebSocket('ws://localhost:8003/subscribe-latest');

    ws.onopen = () => {
    };

    ws.onmessage = (event) => {
        const newImage = JSON.parse(event.data);
        if (imageList)
            imageList.innerHTML = '';

        let i = 0;
       
        newImage.forEach((image: any) => {
            let string = '';

            image['rectangles'].forEach((rect: any) => {
                string += `<rect x=${rect[2]} y=${rect[3]} width=${rect[4] - rect[2]} height=${rect[5] - rect[3]} fill=${rect[6]} />`;
            });

            imageList!.innerHTML += `<svg width=${image['image'][2]} height=${image['image'][3]}>    <rect x="0" y="0" width="${image['image'][2]}" height="${image['image'][3]}" fill="none" stroke="black" stroke-width="2"/>${string}</svg>`;
            i++;
        });

        setTimeout(() => {
            // Code to execute after 10 seconds
        }, 10000);

        postData();
    };  

    ws.onerror = (error) => {
        console.error("WebSocket error:", error);
    };

    ws.onclose = () => {
    };
}

function drawImageAndRect(image: Image, i: number) {
    const editorDiv = document.getElementById('editor')!;
    editorDiv.innerHTML = `
        <canvas id="canvas"></canvas>
        <input type="color" id="colorPicker" value="black">
        <div id="info"></div>
    `;

    const canvas = document.getElementById('canvas') as HTMLCanvasElement;
    const ctx = canvas.getContext('2d') as CanvasRenderingContext2D;
    if (image) {
        canvas.width = image.width;
        canvas.height = image.height;
    } else {
        canvas.width = 500;
        canvas.height = 500;
    }

    let color: string = 'black';

    const colorPicker = document.getElementById('colorPicker') as HTMLInputElement;

    colorPicker.addEventListener('input', () => {
        color = colorPicker.value;
        draw();
    });

    let rectDragging = false;
    let dragged = false;
    let rectStartX = 0;
    let rectStartY = 0;
    let rectEndX = 0;
    let rectEndY = 0;

    function draw() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        if (image) {
            image.rectangles.forEach((rectangle) => {
                ctx.fillStyle = rectangle.color;
                ctx.fillRect(rectangle.x_1, rectangle.y_1, rectangle.x_2 - rectangle.x_1, rectangle.y_2 - rectangle.y_1);
            });
        }
    }

    function onMouseDown(event: MouseEvent) {
        rectDragging = true;

        var rect = canvas.getBoundingClientRect();
        var scaleX = canvas.width / rect.width;
        var scaleY = canvas.height / rect.height;

        const mouseX = (event.clientX - rect.left) * scaleX;
        const mouseY = (event.clientY - rect.top) * scaleY;

        rectStartX = mouseX;
        rectStartY = mouseY;
    }

    function onMouseMove(event: MouseEvent) {
        if (rectDragging) {
            dragged = true;
            draw();
            ctx.fillStyle = color;

            var rect = canvas.getBoundingClientRect();
            var scaleX = canvas.width / rect.width;
            var scaleY = canvas.height / rect.height;

            const mouseX = (event.clientX - rect.left) * scaleX;
            const mouseY = (event.clientY - rect.top) * scaleY;

            ctx.fillRect(rectStartX, rectStartY, mouseX - rectStartX, mouseY - rectStartY);
            rectEndX = mouseX;
            rectEndY = mouseY;
        }
    }

    async function onMouseUp() {
        if (dragged && Math.abs(rectEndX - rectStartX) > 8 && Math.abs(rectEndY - rectStartY) > 8) {
            if (rectEndX < rectStartX) {
                let temp = rectEndX;
                rectEndX = rectStartX;
                rectStartX = temp;
            }

            if (rectEndY < rectStartY) {
                let temp = rectEndY;
                rectEndY = rectStartY;
                rectStartY = temp;
            }

            image?.rectangles.push({
                x_1: rectStartX,
                y_1: rectStartY,
                x_2: rectEndX,
                y_2: rectEndY,
                color: color
            });

            const response = await fetch('http://localhost:8003/rectangle_add/', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    image_id: i,
                    x_1: rectStartX,
                    y_1: rectStartY,
                    x_2: rectEndX,
                    y_2: rectEndY,
                    color: color
                })
            });
        }

        dragged = false;
        rectDragging = false;
        rectEndX = 0;
        rectEndY = 0;
        rectStartX = 0;
        rectStartY = 0;
    }

    function onCanvasClick(event: MouseEvent) {
        var rect = canvas.getBoundingClientRect();
        var scaleX = canvas.width / rect.width;
        var scaleY = canvas.height / rect.height;

        const mouseX = (event.clientX - rect.left) * scaleX;
        const mouseY = (event.clientY - rect.top) * scaleY;

        for (let i = image!.rectangles.length - 1; i >= 0; i--) {
            const rect = image!.rectangles[i];

            if (mouseX >= rect.x_1 && mouseX <= rect.x_2 && mouseY >= rect.y_1 && mouseY <= rect.y_2) {
                to_delete = i;
                const infoDiv = document.getElementById('info')!;
                infoDiv.innerHTML = `Rectangle Info:<br>X: ${rect.x_1} - ${rect.x_2}<br>Y: ${rect.y_1} - ${rect.y_2}<br>Color: ${rect.color}`;
                infoDiv.innerHTML += '<br><button id="delete_button">Delete</button>';
                const button = document.getElementById('delete_button') as HTMLButtonElement;
                button.addEventListener('click', handleButtonClick);

                draw();
                return;
            }
        }

        document.getElementById('info')!.textContent = 'Clicked on empty space';
    }

    canvas.addEventListener('mousedown', onMouseDown);
    canvas.addEventListener('mousemove', onMouseMove);
    canvas.addEventListener('mouseup', onMouseUp);
    canvas.addEventListener('click', onCanvasClick);

    function handleButtonClick() {
        if (to_delete !== -1) {
            const rect_del = image!.rectangles[to_delete];
            image!.rectangles.splice(to_delete, 1);
            to_delete = -1;
            const infoDiv = document.getElementById('info')!;
            infoDiv.innerHTML = '';
            draw();

            const response = fetch('http://localhost:8003/rectangle_delete/', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    image_id: i,
                    x_1: rect_del.x_1,
                    y_1: rect_del.y_1,
                    x_2: rect_del.x_2,
                    y_2: rect_del.y_2,
                    color: rect_del.color
                })
            });
        }
    }

    draw();
}

declare var Promise: {
    new <T>(executor: (resolve: (value?: T | PromiseLike<T>) => void, reject: (reason?: any) => void) => void): Promise<T>;
    resolve(value?: any): Promise<any>;
    reject(reason?: any): Promise<any>;
    all<T>(promises: (T | PromiseLike<T>)[]): Promise<T[]>;
    race<T>(promises: (T | PromiseLike<T>)[]): Promise<T>;
};

function getRandomNumber(min: number, max: number): number {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

const postData = async () => {
    let w = Math.floor(Math.random() * (1000 - 100 + 1)) + 100;
    let h = Math.floor(Math.random() * (1000 - 100 + 1)) + 100;

    let rectangles: Rectangle[] = [];

    const randomNumber = getRandomNumber(5, 6);

    for (let i = 0; i < randomNumber; i++) {
        let x_1 = Math.floor(Math.random() * (w - 1 + 1));
        let y_1 = Math.floor(Math.random() * (h - 1 + 1));
        let x_2 = Math.floor(Math.random() * (w - x_1 + 1)) + x_1;
        let y_2 = Math.floor(Math.random() * (h - y_1 + 1)) + y_1;
        let color = `#${Math.floor(Math.random() * 16777215).toString(16)}`;
        rectangles.push({
            x_1: x_1,
            y_1: y_1,
            x_2: x_2,
            y_2: y_2,
            color: color
        });
    }

    const imageData = {
        title: "Example Image",
        width: w,
        height: h,
        rectangles: rectangles
    };

    const response = await fetch('http://0.0.0.0:8003/images/', {
        method: 'POST',
        body: JSON.stringify(imageData), // Convert JavaScript object to JSON string
        headers: {
            'Content-Type': 'application/json',
        },
    });

    if (!response.ok) {
        const errorMessage = await response.text();
        throw new Error(`HTTP error! Status: ${response.status}, Message: ${errorMessage}`);
    }

    const responseData = await response.json(); // Parse the JSON response
    console.log(responseData); // Output the response data
};

let images: any = [];

async function getData(i: number) {
    const galleryDiv = document.getElementById('gallery')!;

    let area;

    if (document.getElementById(`image_${i}`)) {
        document.getElementById(`try_again_${i}`)?.remove();
        area = document.getElementById(`image_${i}`)!;
        area.innerHTML = '';
        area.innerHTML += `
        <div class="spinner" id="spinner_${i}"></div>
        `;
    } else {
        galleryDiv.innerHTML += `
        <div class="image" id="image_${i}">
        <div class="spinner" id="spinner_${i}"></div>
        </div>
        `;
        area = document.getElementById(`image_${i}`)!;
    }

    area.innerHTML += `<div id="svg_${i}"></div>`;

    area.innerHTML += `<button id="editor_button_${i}" disabled>Edit ${i}</button>`;


    fetch('http://0.0.0.0:8003/images/' + i)
        .then(response => response.json())
        .then(data => {
            let rects: Rectangle[] = [];

            data.rectangles.forEach((rect: any) => {
                const tmp: Rectangle = { x_1: rect[2], y_1: rect[3], x_2: rect[4], y_2: rect[5], color: rect[6] };
                rects.push(tmp);
            });

            let temp: Image = {
                width: data.image[2],
                height: data.image[3],
                rectangles: rects
            }

            let string = '';

            rects.forEach((rect) => {
                string += `<rect x=${rect.x_1} y=${rect.y_1} width=${rect.x_2 - rect.x_1} height=${rect.y_2 - rect.y_1} fill=${rect.color} />`;
            });

            document.getElementById(`svg_${i}`)!.innerHTML = `<svg width=${data.image[2]} height=${data.image[3]}>${string}</svg>`;

            images[i] = temp;

            const button = document.getElementById("editor_button_" + i) as HTMLButtonElement;
            button!.disabled = false;
            const spinnerDiv = document.getElementById(`spinner_${i}`);
            if (spinnerDiv) {
                spinnerDiv.remove();
            }
            button!.addEventListener('click', () => {
                drawImageAndRect(images[i], i);
            });
        })
        .catch(error => {
            console.error('Error:', error);
            const spinnerDiv = document.getElementById(`spinner_${i}`);
            if (spinnerDiv) {
                spinnerDiv.remove();
            }

            const message = document.getElementById(`image_${i}`);
            if (message) {
                message.textContent = 'Error loading image';
                message.innerHTML += `<br><button id="try_again_${i}" >Try Again ${i}</button>`;
                const button = document.getElementById("editor_button_" + i) as HTMLButtonElement;
                if (button)
                    button.remove();
                const tryAgainButton = document.getElementById(`try_again_${i}`) as HTMLButtonElement;

                tryAgainButton.addEventListener('click', () => {
                    fetch('http://0.0.0.0:8003/images/' + i)
                        .then(response => response.json())
                        .then(data => {
                            getData(i);
                        })
                });

            }
        });
}


window.onload = () => {
    const galleryDiv = document.getElementById('gallery')!;

    if (!galleryDiv) {
        latestImages();
        return;
    }

    for (let i = 1; i < 16; i++) {
        getData(i);
    }
};