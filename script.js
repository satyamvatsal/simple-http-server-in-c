function downloadFile() {
    const fileNameInput = document.getElementById('fileName');
    const fileName = fileNameInput.value.trim();

    if (fileName === "") {
        alert("Please enter a file name.");
        return;
    }

    // Replace "YOUR_SERVER_URL/file-to-download.txt" with the actual URL of your server endpoint that handles file downloads.
    const fileDownloadUrl = `${encodeURIComponent(fileName)}`;

    fetch(fileDownloadUrl)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.blob();
        })
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = fileName; // Use the entered file name
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        })
        .catch(error => {
            console.error('Error downloading file:', error);
        });
}
