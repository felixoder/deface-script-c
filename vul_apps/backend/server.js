const express = require("express");
const fs = require("fs");
const path = require("path");

const app = express();
const PORT = 5000;

// Middleware to parse incoming data (raw body for PUT)
app.use(express.raw({ type: "*/*" }));

// Serve uploaded files
app.use("/uploads", express.static(path.join(__dirname, "uploads")));

// Serve the index.html file when accessing the home route
app.get("/", (req, res) => {
    const indexPath = path.join(__dirname, "uploads", "index.html");
    res.sendFile(indexPath, (err) => {
        if (err) {
            res.status(500).send("Error loading the index.html file");
        }
    });
});

// Vulnerable endpoint to handle HTTP PUT
app.put("/:filename", (req, res) => {
    const filename = req.params.filename;
    const uploadPath = path.join(__dirname, "uploads", filename);

    fs.writeFile(uploadPath, req.body, (err) => {
        if (err) {
            console.error("File upload failed:", err);
            return res.status(500).send("Internal Server Error");
        }
        console.log(`File uploaded: ${uploadPath}`);
        res.status(201).send("File uploaded successfully!");
    });
});

// Start the server
app.listen(PORT, () => {
    console.log(`Vulnerable app running at http://localhost:${PORT}`);
});

