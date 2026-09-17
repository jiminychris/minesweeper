const LIGHT_GRAY = 255;
const GRAY = 192;
const DARK_GRAY = 128;

function drawBorder(backbuffer, stride, w, bw, x, y, colorTopLeft, colorCenter, colorBottomRight) {
    let row = y * stride + x * 4;
    const fullW = w + 2*bw;
    for (let j = 0; j < bw; ++j) {
        let at = row;
        let i;
        for (i = 0; i < fullW - j - 1; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = 255;
        for (i = 0; i < j; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
    for (let j = 0; j < w; ++j) {
        let at = row;
        let i;
        for (i = 0; i < bw; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        at += w * 4;
        for (i = 0; i < bw; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
    for (let j = 0; j < bw; ++j) {
        let at = row;
        let i;
        for (i = 0; i < bw - j - 1; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = 255;
        for (i = 0; i < w + bw + j; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
    return bw;
}

function drawSquare(backbuffer, stride, w, bw, x, y) {
    const colorTopLeft = LIGHT_GRAY;
    const colorCenter = GRAY;
    const colorBottomRight = DARK_GRAY;
    let row = y * stride + x * 4;
    const fullW = w + 2*bw;
    for (let j = 0; j < bw; ++j) {
        let at = row;
        let i;
        for (i = 0; i < fullW - j - 1; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = 255;
        for (i = 0; i < j; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
    for (let j = 0; j < w; ++j) {
        let at = row;
        let i;
        for (i = 0; i < bw; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        for (i = 0; i < w; ++i) {
            backbuffer.data[at++] = colorCenter;
            backbuffer.data[at++] = colorCenter;
            backbuffer.data[at++] = colorCenter;
            backbuffer.data[at++] = 255;
        }
        for (i = 0; i < bw; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
    for (let j = 0; j < bw; ++j) {
        let at = row;
        let i;
        for (i = 0; i < bw - j - 1; ++i) {
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = colorTopLeft;
            backbuffer.data[at++] = 255;
        }
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = colorCenter;
        backbuffer.data[at++] = 255;
        for (i = 0; i < w + bw + j; ++i) {
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = colorBottomRight;
            backbuffer.data[at++] = 255;
        }
        row += stride;
    }
}

class Game {
    constructor() {
        this.updateAndRender = this.updateAndRender.bind(this);
    }

    requestNextFrame() {
        this.window.requestAnimationFrame(this.updateAndRender);
    }

    updateAndRender() {
        this.canvas.width = this.window.innerWidth;
        this.canvas.height = this.window.innerHeight;
        const stride = this.canvas.width*4;
        const backbuffer = this.ctx.createImageData(this.canvas.width, this.canvas.height);
        let at = 0;
        for (let y = 0; y < this.canvas.height; ++y) {
            for (let x = 0; x < this.canvas.width; ++x) {
                backbuffer.data[at + 0] = Math.round(255 * (x / this.canvas.width));
                backbuffer.data[at + 1] = Math.round(255 * (1 - y / this.canvas.height));
                backbuffer.data[at + 2] = Math.round(255 * (1 - x / this.canvas.width));
                backbuffer.data[at + 3] = 255;
                at += 4;
            }
        }

        const borderWidth = 2;
        const innerWidth = 12;
        const boardWidth = 9;
        const boardHeight = 9;
        const squareWidth = innerWidth + 2*borderWidth;
        let indent = drawBorder(backbuffer, stride, squareWidth * 9 + 6, 6, 0, 0, GRAY, GRAY, GRAY);
        indent += drawBorder(backbuffer, stride, squareWidth * 9, 3, indent, indent, DARK_GRAY, GRAY, LIGHT_GRAY);

        for (let j = 0; j < boardHeight; ++j) {
            for (let i = 0; i < boardWidth; ++i) {
                drawSquare(backbuffer, stride, innerWidth, borderWidth, indent + i * squareWidth, indent + j * squareWidth);
            }
        }
        this.ctx.fillStyle = 'magenta';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.putImageData(backbuffer, 0, 0);

        this.requestNextFrame();
    }
}

function main() {
    const game = new Game();
    game.canvas = document.getElementById('canvas');
    game.ctx = game.canvas.getContext('2d');
    game.window = window;
    game.requestNextFrame();
}

main();
