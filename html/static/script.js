const form = document.getElementById("postForm");
const target = document.getElementById("target");

const textField = document.getElementById("textField");
const fileField = document.getElementById("fileField");

const textarea = document.getElementById("message");
const fileInput = document.getElementById("file");

function updateForm() {
	if (target.value === "/python/upload.py") {
		form.action = "/python/upload.py";

		textField.style.display = "none";
		fileField.style.display = "block";

		textarea.required = false;
		fileInput.required = true;
	} else {
		form.action = "/python/";

		textField.style.display = "block";
		fileField.style.display = "none";

		textarea.required = true;
		fileInput.required = false;
	}
}

target.addEventListener("change", updateForm);
updateForm();

