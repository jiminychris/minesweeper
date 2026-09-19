const MILLI = 1 / 1000;

class Game {
    constructor() {
        this.updateAndRender = this.updateAndRender.bind(this);
    }

    requestNextFrame() {
        this.window.requestAnimationFrame(this.updateAndRender);
    }

    updateAndRender() {
        const nowSeconds = Date.now() * MILLI;
        const elapsedSeconds = nowSeconds - this.startSeconds;
        this.canvas.width = this.window.innerWidth * this.dpr;
        this.canvas.height = this.window.innerHeight * this.dpr;
        this.canvas.style.width = `${this.window.innerWidth}px`;
        this.canvas.style.height = `${this.window.innerHeight}px`;

        this.instance.exports.GameUpdateAndRender(elapsedSeconds, this.canvas.width, this.canvas.height, this.heap.byteOffset);

        this.ctx.fillStyle = 'magenta';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        const clampedArray = new Uint8ClampedArray(this.memory.buffer, this.heap.byteOffset, this.canvas.width * this.canvas.height * 4);
        this.ctx.putImageData(new ImageData(clampedArray, canvas.width, canvas.height), 0, 0);

        this.requestNextFrame();
    }
}

async function main() {
    const game = new Game();

    game.dpr = 1;
    game.canvas = document.getElementById('canvas');
    game.ctx = game.canvas.getContext('2d');
    game.window = window;
    game.startSeconds = Date.now() * MILLI;

    const stackSize = 64*1024;
    const heapSize = 1024*1024*1024;
    const memorySize = stackSize + heapSize;
    const pages = Math.ceil(memorySize / (64*1024));
    game.memory = new WebAssembly.Memory({ initial: pages, maximum: pages });
    const { instance } = await WebAssembly.instantiateStreaming(fetch("minesweeper.wasm"), {
        env: {
            memory: game.memory,
            cosf: Math.cos,
            roundf: Math.round,
            logu64: console.log,
        },
    });
    game.instance = instance;
    game.heap = new Uint8Array(game.memory.buffer, game.instance.exports.__heap_base, heapSize);

    game.requestNextFrame();
}

main();
