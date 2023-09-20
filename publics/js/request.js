let isDoneA = false;
async function TrySendAuthForm(e) {

    if (isDoneA === true) {
        //isDoneA = false; мб нужное))
        return;
    }

    e.preventDefault();

    rc = await send_authorization_request();
    console.log(rc)
    if (rc.length != 0) {
        isDoneA = true;
        document.getElementById("authorization_form").getElementsByTagName("button")[0].click();
    } else {
        console.log("Wrong pass");
    }
    
}

async function send_authorization_request(){
    let username = document.getElementById("auth_user").value;
    let password = document.getElementById("auth_psw").value;
    let url = "/auth";
    let response = await fetch(url, {
        method: 'POST',
        headers: {
            "Authorization" : "Basic " + username + ":" + password
        },
        body: "aboba",
    });
    let rc = await response.text();
    return rc;
}


let isDoneR = false;
async function TrySendRegForm(e) {

    if (isDoneR === true) {
        //isDoneR = false; мб нужное))
        return;
    }

    e.preventDefault();

    rc = await send_registration_request();
    console.log(rc)
    if (rc.length != 0) {
        isDoneR = true;
        document.getElementById("registration_form").getElementsByTagName("button")[0].click();
    } else {
        console.log("Not avalible");
    }
    
}

async function send_registration_request(){
    let username = document.getElementById("reg_user").value;
    let email    = document.getElementById("reg_email").value;
    let password = document.getElementById("reg_psw").value;
    let url = "/reg";
    let response = await fetch(url, {
        method: 'POST',
        headers: {
            "Registration" : "Basic " + username + ":" + email + ":" + password
        },
        body: "aboba",
    });
    let rc = await response.text();
    return rc;
}

async function SendPostRequest(url, obj) {
    let response = await fetch(url, {
        method: 'POST',
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(obj),
    })
    .then(response => {
        if (!response.ok) {
            return response.json()
                .catch(() => {
                    // Couldn't parse the JSON
                    throw new Error(response.status);
                })
                .then(({message}) => {
                    // Got valid JSON with error response, use it
                    throw new Error(message || response.status);
                });
        }
        // Successful response, parse the JSON and return the data
        return response.json();
    });
    return response;
}

async function createNewProject() {
    let obj = new Object();
    obj.type = "Project";
    obj.data = document.getElementById("project_name_input_field").value;
               document.getElementById("project_name_input_field").value = "";
    let response = await SendPostRequest("/serv", obj);
    if (JSON.stringify(response).length != 0 && response.status == "ok") {
        updateCreatedProjects();
    } else {
        console.log("Ошибка создания карточки")
    }
    return response;
}

async function createNewNote(e) {
    let obj = new Object();
    obj.type = "Note";
    obj.parent = e.name
    if (obj.parent.length == 0) return ""
    obj.data = document.getElementById("note_name_input_field").value;
               document.getElementById("note_name_input_field").value = "";
    let url = "/serv";
    let response = await SendPostRequest("/serv", obj);
    if (JSON.stringify(response).length == 0 || response.status != "ok") {
        console.log("Ошибка создания карточки")
    }
    return response;
}


async function getAllMyProjects() {
    let url = "/serv";
    let response = await fetch(url, {
        method: 'GET',
    })
    .then(response => response.json());
    return response;
}

async function updateCreatedProjects() {
    let el = document.getElementById("created_projects");
    let r = await getAllMyProjects();
    let projects = new Array;
    for (let i of r.projects) {
        projects.push(i[0]);
    }
    projects = projects.filter((item) => !hasChildProject(el, item));
    console.log(projects);
    projects.forEach((item) => pushButtonOfProject(el, item));
}




