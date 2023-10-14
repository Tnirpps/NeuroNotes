function showMainBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.remove("hidden-main-item");
    obj.style.display = "flex";
}

function hideAuthorizationBlock() {
    let obj = document.getElementById("authorization_block");
    obj.classList.add("hidden-form-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    showMainBlock();
}

function hideRegistrationBlock() {
    let obj = document.getElementById("registration_block");
    obj.classList.add("hidden-form-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    showMainBlock();
}

function showRegistrationBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.add("hidden-main-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    let reg = document.getElementById("registration_block");
    reg.classList.remove("hidden-form-item");
    reg.style.display='block';
}

function showAuthorizationBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.add("hidden-main-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    let reg = document.getElementById("authorization_block");
    reg.classList.remove("hidden-form-item");
    reg.style.display='block';
}

function clearActiveNote() {
    let projects = document.getElementById("created_notes").getElementsByTagName("button");
    Array.from(projects).forEach((el) => el.classList.remove("active"));
}

function setActiveNote(el) {
    clearActiveNote();
    el.target.classList.add("active");
    document.getElementById("note_inner_content").style.display = "block";
    document.getElementById("update_note_text").name = el.target.name;
    document.getElementById("remove_note").name = el.target.id;
    document.getElementById("note_text_content").innerHTML = el.target.title;
    CC.user.focus = el.target.id.split("_")[1];
    CC.show();
}

/*
function closeCurrentProject(el) {
    clearActiveProject();
    document.getElementById("create_box").style.display = "block";
    document.getElementById("project_content").style.display = "none";

}
*/

function hasChildProject(parent, name) {
    arr = new Array;
    for (let i of parent.childNodes) {
        if (i.name == name) {
            return true;
        }
    }
    return false;
}

function pushButtonToDOM(parent, text, project, content="", id="") {
    let item = document.createElement("button");
    let maxLen = 15;
    if (project) {
        item.addEventListener("click", setActiveProject);
    } else {
        item.addEventListener("click", setActiveNote);
        item.title = content;
        item.id = ("note_" + id);
    }
    item.textContent = text.length > maxLen?
        text.substr(0, maxLen - 3) + "...":
        text;
    item.name = text
    parent.appendChild(item);

}

function removeAllChidren(el) {
    while (el.lastElementChild) {
        el.removeChild(el.lastElementChild);
    }
}

function setActiveProject(el) {
    document.getElementById("create_box").style.display = "none";
    document.getElementById("note_content").style.display = "block";
    document.getElementById("create_new_note").name = el.target.name;
    document.getElementById("project_box").name = el.target.name;
    updateCreatedNotes(el.target.name);
}

function closeCurrentNote() {
    document.getElementById("create_box").style.display = "block";
    document.getElementById("note_content").style.display = "none";
    document.getElementById("note_inner_content").style.display = "none";
    removeAllChidren(document.getElementById("created_notes"));
    if (Object.keys(CC.graph).length != 0) {
        dumpGraph(CC.graph);
    }
    CC.graph.clear();
    CC.user.focus = -1;
    CC.show();
}

function updateGraphOfNotes(notes, edges) {
    notes.forEach((el) => {
        CC.graph.addNode(el[2], el[3], el[4]);
    });
    console.log(CC.graph.data);
    for (let s of edges)  {
        let edg = s.split(":");
        CC.graph.addEdge(edg[0], edg[1]);
    }
    CC.setData(CC.graph);
    CC.show();
}

function removeNote(e) {
    let s = e.name.split("_");
    CC.graph.removeNode(s[1]);
    CC.show();
    //server remove Node;

}




