class TCanvas {
    constructor(htmlCnv) {
        this.htmlCnv = htmlCnv;
        this.ctx = htmlCnv.getContext('2d');
        this.graph = {};
        this.scale = 1;
        this.DPC = 40;
        this.user = {
            select: false,
            active: -1,
            focus: -1,
            camPos : {
                x: 0,
                y: 0,
            },
        }

        /* ====================== drag ======================= */
        this.BB = htmlCnv.getBoundingClientRect();
        this.drag = {
            is: false,
            x: 0,
            y: 0,
            active: 0,
        };


    }

    moveX(stepX) {
        this.user.camPos.x -= stepX;
        this.show();
    }

    setData(graph) {
        this.graph = graph;
        for (let i in this.graph.data) {
            let u = this.graph.data[i];
            this.graph.data[i].x = Math.round(u.x / this.DPC) * this.DPC;
            this.graph.data[i].y = Math.round(u.y / this.DPC) * this.DPC;
        }
    }

    clear() {
        this.ctx.clearRect(0, 0, this.htmlCnv.width, this.htmlCnv.height);
    }

    show() {
        this.clear();
        if (Object.keys(this.graph).length != 0) {
            this.graph.render(this.ctx, this.user);
        }
    }

    async myDownRight(e) {
        if (!this.user.select) return;
        const offsetX = this.BB.left;
        const offsetY = this.BB.top;
        const mx = parseInt(e.clientX - offsetX);
        const my = parseInt(e.clientY - offsetY);

        let onNode = false;
        for (const i in this.graph.data) {
            let s = this.graph.data[i];
            const dx = s.x - mx;
            const dy = s.y - my;
            // test if the mouse is inside this circle
            if (dx * dx + dy * dy < s.radius * s.radius) {
                let u = i;
                let v = this.user.active;
                if (this.graph.hasEdge(u, v)) {
                    let r = await sendRemoveEdge(u, v);
                    if (r.status != "ok") return;
                }
                this.graph.removeEdge(u, v);
                this.show();
                break;
            }
        }
    }

    myDownLeft(e) {
        const offsetX = this.BB.left;
        const offsetY = this.BB.top;
        const mx = parseInt(e.clientX - offsetX);
        const my = parseInt(e.clientY - offsetY);

        this.drag.is = false;
        let onNode = false;
        for (const i in this.graph.data) {
            let s = this.graph.data[i];
            const dx = s.x - mx;
            const dy = s.y - my;
            // test if the mouse is inside this circle
            if (!this.drag.is && dx * dx + dy * dy < s.radius * s.radius) {
                onNode = true;
                let u = i;
                let v = this.user.active;
                if (this.user.select && u != v && !this.graph.hasEdge(u, v)) {
                    this.graph.addEdge(u, v);
                    this.user.select = false;
                    this.drag.is = false;
                } else {
                    this.user.select = true;
                    this.user.active = i;
                    this.drag.is = true;
                    this.drag.active = i;
                }
                break;
            }
        }
        this.user.select = onNode;
        this.show();
        // save the current mouse position
        this.drag.x = mx;
        this.drag.y = my;
    }

    myUp(e) {
        this.drag.is = false;
        if (Object.keys(this.graph).length == 0) return;
        if (this.graph.data.hasOwnProperty(this.drag.active)) {
            let u = this.graph.data[this.drag.active]; 
            this.graph.data[this.drag.active].x = Math.round(u.x / this.DPC) * this.DPC;
            this.graph.data[this.drag.active].y = Math.round(u.y / this.DPC) * this.DPC;
            this.show();
        }
    }

    myMove(e) {
        if (!this.drag.is) return;
        // tell the browser we're handling this mouse event

        const offsetX = this.BB.left;
        const offsetY = this.BB.top;
        // get the current mouse position
        const mx = parseInt(e.clientX - offsetX);
        const my = parseInt(e.clientY - offsetY);

        // calculate the distance the mouse has moved
        // since the last mousemove
        const dx = mx - this.drag.x;
        const dy = my - this.drag.y;

        // move each rect that isDragging
        // by the distance the mouse has moved
        // since the last mousemove
        if (this.graph.data.hasOwnProperty(this.drag.active)) {
            let free = true;
            for (const i in this.graph.data) {
                if (i == this.drag.active) {
                    continue;
                }
                let u = this.graph.data[i];
                let X = this.graph.data[this.drag.active].x + dx;
                let Y = this.graph.data[this.drag.active].y + dy;
                if ((u.x - X) * (u.x - X) + (u.y - Y) * (u.y - Y) < 2 * u.radius * 2 * u.radius) {
                    free = false;
                    break;
                }

            }
            if (free) {
                this.graph.data[this.drag.active].x += dx;
                this.graph.data[this.drag.active].y += dy;
            }

        }

        // redraw the scene with the new rect positions
        this.show();

        // reset the starting mouse position for the next mousemove
        this.drag.x = mx;
        this.drag.y = my;
    }

}

class TGraph {
    constructor(activeNodeColor, nodeColor, edgeColor) {
        this.data = {};
        this.graph = {};
        this.edgeColor = edgeColor;
        this.nodeColor = nodeColor;
        this.activeNodeColor = activeNodeColor;
    }

    removeEdge(u, v) {
        if (!this.hasEdge(u, v)) return;
        if (this.graph.hasOwnProperty(u)) {
            const index = this.graph[u].indexOf(v);
            if (index > -1) {
                this.graph[u].splice(index, 1);
            }
        }
        if (this.graph.hasOwnProperty(v)) {
            const index = this.graph[v].indexOf(u);
            if (index > -1) {
                this.graph[v].splice(index, 1);
            }
        }
    }
    
    addEdge(u, v) {
        if (this.hasEdge(u, v)) return;
        let f = false;
        for (let i = 0; i < this.graph[u].length; i++) {
            if (this.graph[u][i].id == v) {
                f = true;
            }
        }
        if (!f) {
            this.graph[u].push(v);
        }
    }

    hasEdge(u, v) {
        if (this.graph.hasOwnProperty(u)) {
            for (const e of this.graph[u]) {
                if (e == v) return true;
            };
        }
        if (this.graph.hasOwnProperty(v)) {
            for (const e of this.graph[v]) {
                if (e == u) return true;
            };
        }
        return false;
    }

    addNode(id, x, y) {
        if (this.data.hasOwnProperty(id)) {
            return;
        }
        this.data[id] = new TNode(id, x, y, 20);
        this.graph[id] = new Array();
    }

    removeNode(id) {
        delete this.data[id];
        for (const i in this.graph) {
            if (i == id) {
                delete this.graph[i];   
                continue;
            }
            this.graph[i] = this.graph[i].filter( el => el != id);
        }
    }

    render(ctx, user) {
        for (const i in this.graph) {
            for (let j = 0; j < this.graph[i].length; j++) {
                this.renderEdge(ctx, this.data[i], this.data[this.graph[i][j]], user.camPos);
            }
        }
        for (const id in this.data) {
            if (user.select && user.active == id) {
                this.data[id].render(ctx, user.camPos, this.activeNodeColor, user.focus);
            } else {
                this.data[id].render(ctx, user.camPos, this.nodeColor, user.focus);
            }
        }
    }

    renderEdge(ctx, u, v, camPos) {
        ctx.beginPath();
        ctx.moveTo(u.x - camPos.x, u.y - camPos.y);
        ctx.lineTo(v.x - camPos.x, v.y - camPos.y);
        ctx.lineWidth = 5;
        ctx.strokeStyle = this.edgeColor;
        ctx.stroke();
    }

    dumpNodes() {
        let r = ""
        for (const id in this.data) {
            r += this.data[id].asStr() + ","
        }
        return r;
    }

    dumpEdges() {
        let r = ""
        for (const i in this.graph) {
            for (let j = 0; j < this.graph[i].length; j++) {
                r += this.data[i].id + ":" + this.data[this.graph[i][j]].id + ",";
            }
        }
        return r;
    }

    clear() {
        this.data = {};
        this.graph = {};
    }
}


class TNode {
    constructor(id, posX, posY, radius) {
        this.id = id;
        this.x = posX;
        this.y = posY;
        this.radius = radius;
    }

    render(ctx, camPos, color, focus) {
        if (this.id != focus) {
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(this.x - camPos.x, this.y - camPos.y, this.radius, 0, 2 * Math.PI);
            //htmlCnv.ctx.lineWidth = 10;
            //htmlCnv.ctx.strokeStyle = "blue";
            //htmlCnv.ctx.stroke();
            ctx.fill();
        } else {
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(this.x - camPos.x, this.y - camPos.y, this.radius * 1.2, 0, 2 * Math.PI);
            ctx.lineWidth = 10;
            ctx.strokeStyle = "blue";
            ctx.stroke();
            ctx.fill();

        }
    }

    asStr() {
        return (this.id + "|" + this.x + ":" + this.y);
    }
}



