class TCanvas {
    constructor(htmlCnv) {
        this.ctx = htmlCnv.getContext("2d");
        this.scale = 1;
        this.data = new Array();
    }

    pushNode(posX, posY, radius, color="#00000000") {
        this.data.push(new TNode(posX, posY, radius, color));
    }

    update(){
        this.data.forEach((el) => {
            el.render(this);
        })
    }
}

class TNode {
    static idCount = 0;
    constructor(posX, posY, radius, color) {
        this.x = posX;
        this.y = posY;
        this.id = TNode.idCount++;
        this.radius = radius;
        this.color = color;
    }

    render(htmlCnv) {
        htmlCnv.ctx.fillStyle = this.color;
        htmlCnv.ctx.beginPath();
        htmlCnv.ctx.arc(this.x, this.y, this.radius, 0, 2 * Math.PI);
        htmlCnv.ctx.stroke();
        htmlCnv.ctx.fill();
    }
}

function drawHandler(e) {
    let rect = c.ctx.canvas.getBoundingClientRect();
    c.pushNode(e.clientX - rect.left, e.clientY - rect.top, 20, "#FCBA03");
    c.update();
}



