const MILLI = 1 / 1000;

class Game {
    constructor() {
        this.leftMouseButton = {
            halfTransitionCount: 0,
            endedDown: false,
        };
        this.rightMouseButton = {
            halfTransitionCount: 0,
            endedDown: false,
        };
        this.updateAndRender = this.updateAndRender.bind(this);
        this.onMouseMove = this.onMouseMove.bind(this);
        this.onMouseUp = this.onMouseUp.bind(this);
        this.onMouseDown = this.onMouseDown.bind(this);
    }

    requestNextFrame() {
        this.window.requestAnimationFrame(this.updateAndRender);
    }

    onMouseMove(e) {
        this.mouseX = e.clientX;
        this.mouseY = e.clientY;
    }

    onMouseUp(e) {
        let button;
        switch (e.button)
        {
            case 0:
            {
                button = this.leftMouseButton;
            } break;
            case 2:
            {
                button = this.rightMouseButton;
            } break;
        }
        if (button) {
            button.halfTransitionCount++;
            button.endedDown = false;
        }
    }

    onMouseDown(e) {
        let button;
        switch (e.button)
        {
            case 0:
            {
                button = this.leftMouseButton;
            } break;
            case 2:
            {
                button = this.rightMouseButton;
            } break;
        }
        if (button) {
            button.halfTransitionCount++;
            button.endedDown = true;
        }
    }

    updateAndRender() {
        const nowSeconds = Date.now() * MILLI;
        const elapsedSeconds = nowSeconds - this.startSeconds;
        this.canvas.width = this.window.innerWidth * this.dpr;
        this.canvas.height = this.window.innerHeight * this.dpr;
        this.canvas.style.width = `${this.window.innerWidth}px`;
        this.canvas.style.height = `${this.window.innerHeight}px`;

        console.log(this.mouseX, this.mouseY);
        this.gameMemoryView.setInt32(0, this.mouseX, true);
        this.gameMemoryView.setInt32(4, this.mouseY, true);
        this.gameMemoryView.setUint32(8, this.leftMouseButton.endedDown, true);
        this.gameMemoryView.setUint32(12, this.leftMouseButton.halfTransitionCount, true);
        this.gameMemoryView.setUint32(16, this.rightMouseButton.endedDown, true);
        this.gameMemoryView.setUint32(20, this.rightMouseButton.halfTransitionCount, true);

        this.instance.exports.GameUpdateAndRender(elapsedSeconds, this.canvas.width, this.canvas.height, this.heap.byteOffset, this.assetsMemory.byteOffset, this.gameMemory.length, this.gameMemory.byteOffset);
        this.leftMouseButton.halfTransitionCount = 0;
        this.rightMouseButton.halfTransitionCount = 0;

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
    const { instance } = await WebAssembly.instantiateStreaming(fetch('minesweeper.wasm'), {
        env: {
            memory: game.memory,
            cosf: Math.cos,
            roundf: Math.round,
            logu64: console.log,
        },
    });
    game.instance = instance;

    let allocated = game.instance.exports.__heap_base;
    game.assetsMemory = new Uint8Array(game.memory.buffer, allocated, 20*1024*1024);
    allocated += game.assetsMemory.length;
    game.gameMemory = new Uint8Array(game.memory.buffer, allocated, 2*1024*1024);
    game.gameMemoryView = new DataView(game.gameMemory.buffer, game.gameMemory.byteOffset, game.gameMemory.byteLength);
    allocated += game.gameMemory.length;
    game.heap = new Uint8Array(game.memory.buffer, allocated, heapSize - allocated);
    allocated += game.heap.length;
    fetch('assets.msp').then(async response => {
        game.assetsMemory.set(new Uint8Array(await response.arrayBuffer()));
    });

    game.canvas.addEventListener('mousemove', game.onMouseMove);
    game.canvas.addEventListener('mouseup', game.onMouseUp);
    game.canvas.addEventListener('mousedown', game.onMouseDown);
    game.canvas.addEventListener('contextmenu', e => {
        e.preventDefault();
    });

    game.requestNextFrame();
}

main();
