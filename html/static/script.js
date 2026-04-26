const form = document.getElementById("postForm");
const target = document.getElementById("target");

const textField = document.getElementById("textField");
const fileField = document.getElementById("fileField");
const nameField = document.getElementById("nameField");

const message = document.getElementById("message");
const file = document.getElementById("file");
const username = document.getElementById("username");

function enable(el, state) {
	el.disabled = !state;
	if (!state) el.value = "";
}

function updateForm() {
	const value = target.value;
	form.action = value;

	if (value.includes("upload")) {
		textField.style.display = "none";
		fileField.style.display = "block";
		nameField.style.display = "block";

		enable(message, false);
		enable(file, true);
		enable(username, true);
	} else if (value.includes("capitalize")) {
		textField.style.display = "block";
		fileField.style.display = "none";
		nameField.style.display = "none";

		enable(message, true);
		enable(file, false);
		enable(username, false);
	}
	else {
		textField.style.display = "none";
		fileField.style.display = "none";
		nameField.style.display = "none";

		enable(message, false);
		enable(file, false);
		enable(username, false);
	}

}

target.addEventListener("change", updateForm);
updateForm();
