var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var _this = this;
var to_delete = -1;
var exampleImage = {
    width: 800,
    height: 600,
    rectangles: [
        {
            x_1: 100,
            y_1: 100,
            x_2: 200,
            y_2: 200,
            color: 'red'
        },
        {
            x_1: 300,
            y_1: 300,
            x_2: 400,
            y_2: 400,
            color: 'blue'
        },
    ]
};
function latestImages() {
    var imageList = document.getElementById('gallery2');
    var ws = new WebSocket('ws://localhost:8003/subscribe-latest');
    ws.onopen = function () {
    };
    ws.onmessage = function (event) {
        var newImage = JSON.parse(event.data);
        if (imageList)
            imageList.innerHTML = '';
        var i = 0;
        newImage.forEach(function (image) {
            var string = '';
            image['rectangles'].forEach(function (rect) {
                string += "<rect x=".concat(rect[2], " y=").concat(rect[3], " width=").concat(rect[4] - rect[2], " height=").concat(rect[5] - rect[3], " fill=").concat(rect[6], " />");
            });
            imageList.innerHTML += "<svg width=".concat(image['image'][2], " height=").concat(image['image'][3], ">    <rect x=\"0\" y=\"0\" width=\"").concat(image['image'][2], "\" height=\"").concat(image['image'][3], "\" fill=\"none\" stroke=\"black\" stroke-width=\"2\"/>").concat(string, "</svg>");
            i++;
        });
        setTimeout(function () {
            // Code to execute after 10 seconds
        }, 10000);
        postData();
    };
    ws.onerror = function (error) {
        console.error("WebSocket error:", error);
    };
    ws.onclose = function () {
    };
}
function drawImageAndRect(image, i) {
    var editorDiv = document.getElementById('editor');
    editorDiv.innerHTML = "\n        <canvas id=\"canvas\"></canvas>\n        <input type=\"color\" id=\"colorPicker\" value=\"black\">\n        <div id=\"info\"></div>\n    ";
    var canvas = document.getElementById('canvas');
    var ctx = canvas.getContext('2d');
    if (image) {
        canvas.width = image.width;
        canvas.height = image.height;
    }
    else {
        canvas.width = 500;
        canvas.height = 500;
    }
    var color = 'black';
    var colorPicker = document.getElementById('colorPicker');
    colorPicker.addEventListener('input', function () {
        color = colorPicker.value;
        draw();
    });
    var rectDragging = false;
    var dragged = false;
    var rectStartX = 0;
    var rectStartY = 0;
    var rectEndX = 0;
    var rectEndY = 0;
    function draw() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        if (image) {
            image.rectangles.forEach(function (rectangle) {
                ctx.fillStyle = rectangle.color;
                ctx.fillRect(rectangle.x_1, rectangle.y_1, rectangle.x_2 - rectangle.x_1, rectangle.y_2 - rectangle.y_1);
            });
        }
    }
    function onMouseDown(event) {
        rectDragging = true;
        var rect = canvas.getBoundingClientRect();
        var scaleX = canvas.width / rect.width;
        var scaleY = canvas.height / rect.height;
        var mouseX = (event.clientX - rect.left) * scaleX;
        var mouseY = (event.clientY - rect.top) * scaleY;
        rectStartX = mouseX;
        rectStartY = mouseY;
    }
    function onMouseMove(event) {
        if (rectDragging) {
            dragged = true;
            draw();
            ctx.fillStyle = color;
            var rect = canvas.getBoundingClientRect();
            var scaleX = canvas.width / rect.width;
            var scaleY = canvas.height / rect.height;
            var mouseX = (event.clientX - rect.left) * scaleX;
            var mouseY = (event.clientY - rect.top) * scaleY;
            ctx.fillRect(rectStartX, rectStartY, mouseX - rectStartX, mouseY - rectStartY);
            rectEndX = mouseX;
            rectEndY = mouseY;
        }
    }
    function onMouseUp() {
        return __awaiter(this, void 0, void 0, function () {
            var temp, temp, response;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!(dragged && Math.abs(rectEndX - rectStartX) > 8 && Math.abs(rectEndY - rectStartY) > 8)) return [3 /*break*/, 2];
                        if (rectEndX < rectStartX) {
                            temp = rectEndX;
                            rectEndX = rectStartX;
                            rectStartX = temp;
                        }
                        if (rectEndY < rectStartY) {
                            temp = rectEndY;
                            rectEndY = rectStartY;
                            rectStartY = temp;
                        }
                        image === null || image === void 0 ? void 0 : image.rectangles.push({
                            x_1: rectStartX,
                            y_1: rectStartY,
                            x_2: rectEndX,
                            y_2: rectEndY,
                            color: color
                        });
                        return [4 /*yield*/, fetch('http://localhost:8003/rectangle_add/', {
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
                            })];
                    case 1:
                        response = _a.sent();
                        _a.label = 2;
                    case 2:
                        dragged = false;
                        rectDragging = false;
                        rectEndX = 0;
                        rectEndY = 0;
                        rectStartX = 0;
                        rectStartY = 0;
                        return [2 /*return*/];
                }
            });
        });
    }
    function onCanvasClick(event) {
        var rect = canvas.getBoundingClientRect();
        var scaleX = canvas.width / rect.width;
        var scaleY = canvas.height / rect.height;
        var mouseX = (event.clientX - rect.left) * scaleX;
        var mouseY = (event.clientY - rect.top) * scaleY;
        for (var i_1 = image.rectangles.length - 1; i_1 >= 0; i_1--) {
            var rect_1 = image.rectangles[i_1];
            if (mouseX >= rect_1.x_1 && mouseX <= rect_1.x_2 && mouseY >= rect_1.y_1 && mouseY <= rect_1.y_2) {
                to_delete = i_1;
                var infoDiv = document.getElementById('info');
                infoDiv.innerHTML = "Rectangle Info:<br>X: ".concat(rect_1.x_1, " - ").concat(rect_1.x_2, "<br>Y: ").concat(rect_1.y_1, " - ").concat(rect_1.y_2, "<br>Color: ").concat(rect_1.color);
                infoDiv.innerHTML += '<br><button id="delete_button">Delete</button>';
                var button = document.getElementById('delete_button');
                button.addEventListener('click', handleButtonClick);
                draw();
                return;
            }
        }
        document.getElementById('info').textContent = 'Clicked on empty space';
    }
    canvas.addEventListener('mousedown', onMouseDown);
    canvas.addEventListener('mousemove', onMouseMove);
    canvas.addEventListener('mouseup', onMouseUp);
    canvas.addEventListener('click', onCanvasClick);
    function handleButtonClick() {
        if (to_delete !== -1) {
            var rect_del = image.rectangles[to_delete];
            image.rectangles.splice(to_delete, 1);
            to_delete = -1;
            var infoDiv = document.getElementById('info');
            infoDiv.innerHTML = '';
            draw();
            var response = fetch('http://localhost:8003/rectangle_delete/', {
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
function getRandomNumber(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}
var postData = function () { return __awaiter(_this, void 0, void 0, function () {
    var w, h, rectangles, randomNumber, i, x_1, y_1, x_2, y_2, color, imageData, response, errorMessage, responseData;
    return __generator(this, function (_a) {
        switch (_a.label) {
            case 0:
                w = Math.floor(Math.random() * (1000 - 100 + 1)) + 100;
                h = Math.floor(Math.random() * (1000 - 100 + 1)) + 100;
                rectangles = [];
                randomNumber = getRandomNumber(5, 6);
                for (i = 0; i < randomNumber; i++) {
                    x_1 = Math.floor(Math.random() * (w - 1 + 1));
                    y_1 = Math.floor(Math.random() * (h - 1 + 1));
                    x_2 = Math.floor(Math.random() * (w - x_1 + 1)) + x_1;
                    y_2 = Math.floor(Math.random() * (h - y_1 + 1)) + y_1;
                    color = "#".concat(Math.floor(Math.random() * 16777215).toString(16));
                    rectangles.push({
                        x_1: x_1,
                        y_1: y_1,
                        x_2: x_2,
                        y_2: y_2,
                        color: color
                    });
                }
                imageData = {
                    title: "Example Image",
                    width: w,
                    height: h,
                    rectangles: rectangles
                };
                return [4 /*yield*/, fetch('http://0.0.0.0:8003/images/', {
                        method: 'POST',
                        body: JSON.stringify(imageData),
                        headers: {
                            'Content-Type': 'application/json'
                        }
                    })];
            case 1:
                response = _a.sent();
                if (!!response.ok) return [3 /*break*/, 3];
                return [4 /*yield*/, response.text()];
            case 2:
                errorMessage = _a.sent();
                throw new Error("HTTP error! Status: ".concat(response.status, ", Message: ").concat(errorMessage));
            case 3: return [4 /*yield*/, response.json()];
            case 4:
                responseData = _a.sent();
                console.log(responseData); // Output the response data
                return [2 /*return*/];
        }
    });
}); };
var images = [];
function getData(i) {
    var _a;
    return __awaiter(this, void 0, void 0, function () {
        var galleryDiv, area;
        return __generator(this, function (_b) {
            galleryDiv = document.getElementById('gallery');
            if (document.getElementById("image_".concat(i))) {
                (_a = document.getElementById("try_again_".concat(i))) === null || _a === void 0 ? void 0 : _a.remove();
                area = document.getElementById("image_".concat(i));
                area.innerHTML = '';
                area.innerHTML += "\n        <div class=\"spinner\" id=\"spinner_".concat(i, "\"></div>\n        ");
            }
            else {
                galleryDiv.innerHTML += "\n        <div class=\"image\" id=\"image_".concat(i, "\">\n        <div class=\"spinner\" id=\"spinner_").concat(i, "\"></div>\n        </div>\n        ");
                area = document.getElementById("image_".concat(i));
            }
            area.innerHTML += "<div id=\"svg_".concat(i, "\"></div>");
            area.innerHTML += "<button id=\"editor_button_".concat(i, "\" disabled>Edit ").concat(i, "</button>");
            fetch('http://0.0.0.0:8003/images/' + i)
                .then(function (response) { return response.json(); })
                .then(function (data) {
                var rects = [];
                data.rectangles.forEach(function (rect) {
                    var tmp = { x_1: rect[2], y_1: rect[3], x_2: rect[4], y_2: rect[5], color: rect[6] };
                    rects.push(tmp);
                });
                var temp = {
                    width: data.image[2],
                    height: data.image[3],
                    rectangles: rects
                };
                var string = '';
                rects.forEach(function (rect) {
                    string += "<rect x=".concat(rect.x_1, " y=").concat(rect.y_1, " width=").concat(rect.x_2 - rect.x_1, " height=").concat(rect.y_2 - rect.y_1, " fill=").concat(rect.color, " />");
                });
                document.getElementById("svg_".concat(i)).innerHTML = "<svg width=".concat(data.image[2], " height=").concat(data.image[3], ">").concat(string, "</svg>");
                images[i] = temp;
                var button = document.getElementById("editor_button_" + i);
                button.disabled = false;
                var spinnerDiv = document.getElementById("spinner_".concat(i));
                if (spinnerDiv) {
                    spinnerDiv.remove();
                }
                button.addEventListener('click', function () {
                    drawImageAndRect(images[i], i);
                });
            })["catch"](function (error) {
                console.error('Error:', error);
                var spinnerDiv = document.getElementById("spinner_".concat(i));
                if (spinnerDiv) {
                    spinnerDiv.remove();
                }
                var message = document.getElementById("image_".concat(i));
                if (message) {
                    message.textContent = 'Error loading image';
                    message.innerHTML += "<br><button id=\"try_again_".concat(i, "\" >Try Again ").concat(i, "</button>");
                    var button = document.getElementById("editor_button_" + i);
                    if (button)
                        button.remove();
                    var tryAgainButton = document.getElementById("try_again_".concat(i));
                    tryAgainButton.addEventListener('click', function () {
                        fetch('http://0.0.0.0:8003/images/' + i)
                            .then(function (response) { return response.json(); })
                            .then(function (data) {
                            getData(i);
                        });
                    });
                }
            });
            return [2 /*return*/];
        });
    });
}
window.onload = function () {
    var galleryDiv = document.getElementById('gallery');
    if (!galleryDiv) {
        latestImages();
        return;
    }
    for (var i = 1; i < 16; i++) {
        getData(i);
    }
};
