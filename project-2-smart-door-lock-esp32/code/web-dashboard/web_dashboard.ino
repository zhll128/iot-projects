

int getHariDariTimestamp(String ts) {
  // format: DD-MM-YYYY HH:MM:SS
  // sementara simulasi: Senin = 0
  int day = ts.substring(0,2).toInt();
  return (day % 5); // masih simulasi, tapi stabil
}

// Fungsi untuk serve halaman utama (dashboard lengkap)
void handleRoot() {
  String html = "<html><head><title>Dashboard Absensi</title>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
html += "<style>";
html += "body{font-family:Arial;margin:0;padding:10px;background:#f9f9f9}";
html += ".container{max-width:1200px;margin:auto;background:white;padding:15px;border-radius:8px}";
html += "table{border-collapse:collapse;width:100%;min-width:600px}";
html += "th,td{border:1px solid #ddd;padding:8px;font-size:14px;text-align:center}";
html += "th{background:#f2f2f2}";
html += "h1,h2,h3{text-align:center}";
html += ".table-scroll{width:100%;overflow-x:auto}";
html += ".table-scroll table{min-width:700px}";
html += ".main-title{font-size:28px;font-weight:bold;margin-bottom:20px}";
html += ".manage-user{text-align:center;max-width:400px;margin:20px auto}";
html += ".manage-user input,.manage-user select{width:100%;margin:6px 0;padding:6px}";
html += "@media(max-width:768px){";
html += "th,td{font-size:12px;padding:6px}";
html += "}";
html += "</style></head><body>";
html += "<div class='container'>";
html += "<h1 class='main-title'>RFID ATTENDANCE</h1>";
  
  // Log Terakhir
  html += "<h2>Log Terakhir</h2>";

if (logIndex > 0) {
  int lastIndex = (logIndex - 1 + 100) % 100;

  html += "<table style='max-width:400px;margin:auto'>";
  html += "<tr><td><b>UID</b></td><td>" + logs[lastIndex].uid + "</td></tr>";
  html += "<tr><td><b>Nama</b></td><td>" + logs[lastIndex].name + "</td></tr>";
  html += "<tr><td><b>Role</b></td><td>" + logs[lastIndex].role + "</td></tr>";
  html += "<tr><td><b>Status</b></td><td>" + logs[lastIndex].status + "</td></tr>";
  html += "<tr><td><b>Waktu</b></td><td>" + logs[lastIndex].timestamp + "</td></tr>";
  html += "</table>";
} else {
  html += "<p style='text-align:center'>Tidak ada log.</p>";
}
  
// ===== ACCESS TREND (HORIZONTAL BAR) =====
html += "<h2>Access Trend</h2>";

int validCount = 0;
int invalidCount = 0;

for (int i = 0; i < 100; i++) {
  if (logs[i].status == "granted") validCount++;
  else if (logs[i].status == "denied") invalidCount++;
}

int total = validCount + invalidCount;
if (total == 0) total = 1;

// hitung panjang bar (max 100%)
int validWidth = (validCount * 100) / total;
int invalidWidth = 100 - validWidth;

// BAR
html += "<div style='width:100%;height:22px;background:#ddd;border-radius:6px;overflow:hidden;display:flex'>";
html += "<div style='width:" + String(validWidth) + "%;background:#4CAF50'></div>";
html += "<div style='width:" + String(invalidWidth) + "%;background:#F44336'></div>";
html += "</div>";

// TEKS JUMLAH
html += "<div style='margin-top:6px;font-size:14px;text-align:center'>";
html += "<span style='color:#4CAF50;font-weight:bold'>Valid: " + String(validCount) + "</span>";
html += " | ";
html += "<span style='color:#F44336;font-weight:bold'>Invalid: " + String(invalidCount) + "</span>";
html += "</div>";


  // Daftar UID Terdaftar dan Rolenya
  html += "<h2>Daftar UID Terdaftar</h2>";
  html += "<div class='table-scroll'>";
  html += "<table><tr><th>NO.</th><th>NAMA</th><th>UID</th><th>ROLE</th><th>Jumlah Scan Valid</th><th>Action</th></tr>";
  for (int i = 0; i < numUsers; i++) {
    int scanCount = 0;
    for (int j = 0; j < 100; j++) {
      if (logs[j].uid == users[i].uid && logs[j].status == "granted") scanCount++;
    }
    html += "<tr><td>" + String(i+1) + "</td><td>" + users[i].name + "</td><td>" + users[i].uid + "</td><td>" + users[i].role + "</td><td>" + String(scanCount) + "</td>";
    html += "<td><a href='/edituser?index=" + String(i) + "'>Edit</a> | <a href='/deleteuser?index=" + String(i) + "'>Delete</a></td></tr>";
  }
  html += "</table></div>";
  
  // Manage Users (CRUD - Tambah)
  html += "<div class='manage-user'>";
  html += "<h2>Manage Users - Tambah User</h2>";
  html += "<form action='/adduser' method='POST'>";
  html += "UID: <input type='text' name='uid' required><br>";
  html += "Name: <input type='text' name='name' required><br>";
  html += "Role: <select name='role'><option value='siswa'>Siswa</option><option value='guru_mapel'>Guru Mapel</option><option value='wali_kelas'>Wali Kelas</option><option value='admin'>Admin</option></select><br>";
  html += "<input type='submit' value='Tambah User'></form>";
  
  // View Logs (dengan filter sederhana)
  html += "<h2>View Logs</h2><form action='/logs' method='GET'>";
  html += "Filter Status: <select name='status'><option value=''>All</option><option value='granted'>Granted</option><option value='denied'>Denied</option></select> ";
  html += "<input type='submit' value='Filter'></form>";
  html += "<div class='table-scroll'>";
  html += "<table><tr><th>NO.</th><th>UID</th><th>NAMA</th><th>ROLE</th><th>STATUS</th><th>TIMESTAMP</th></tr>";
  String filterStatus = server.arg("status");
  for (int i = 0; i < 100; i++) {
    if (logs[i].uid != "" && (filterStatus == "" || logs[i].status == filterStatus)) {
      html += "<tr><td>" + String(i+1) + "</td><td>" + logs[i].uid + "</td><td>" + logs[i].name + "</td><td>" + logs[i].role + "</td><td>" + logs[i].status + "</td><td>" + logs[i].timestamp + "</td></tr>";
    }
  }
  html += "</table></div>";
  
  // Table Absensi Siswa (mirip spreadsheets)
html += "<h2>Absensi Siswa (Bulanan)</h2>";
html += "<div style='overflow-x:auto'>";
html += "<table><tr><th>NO</th><th>Nama</th><th>UID</th>";
for (int d = 1; d <= 31; d++) html += "<th>" + String(d) + "</th>";
html += "<th>Total</th></tr>";

for (int i = 0; i < numUsers; i++) {
  if (users[i].role == "siswa") {
    html += "<tr><td>" + String(i+1) + "</td><td>" + users[i].name + "</td><td>" + users[i].uid + "</td>";

    int hadir = 0;
    for (int d = 1; d <= 31; d++) {
      bool hadirHariIni = false;

      for (int l = 0; l < 100; l++) {
        if (logs[l].uid == users[i].uid && logs[l].status == "granted") {
          if (logs[l].timestamp.substring(0, 2).toInt() == d) {
            hadirHariIni = true;
            break;
          }
        }
      }

      if (hadirHariIni) {
        html += "<td>H</td>";
        hadir++;
      } else {
        html += "<td>-</td>";
      }
    }

    html += "<td>" + String(hadir) + "</td></tr>";
  }
}
html += "</table>";
html += "</div>";

  
  // Table Absensi Guru (mirip spreadsheets)
html += "<h2>Jurnal Guru (Mingguan)</h2>";
html += "<div style='overflow-x:auto'>";
html += "<table><tr><th>NO</th><th>Nama</th><th>UID</th>";
html += "<th>Senin</th><th>Selasa</th><th>Rabu</th><th>Kamis</th><th>Jumat</th><th>Total</th></tr>";

for (int i = 0; i < numUsers; i++) {
  if (users[i].role == "guru_mapel") {
    bool hadir[5] = {false, false, false, false, false};

    // cek log
    for (int l = 0; l < 100; l++) {
      if (logs[l].uid == users[i].uid && logs[l].status == "granted") {
        int hari = getHariDariTimestamp(logs[l].timestamp);
        hadir[hari] = true;
      }
    }

    int total = 0;
    html += "<tr><td>" + String(i+1) + "</td>";
    html += "<td>" + users[i].name + "</td>";
    html += "<td>" + users[i].uid + "</td>";

    for (int d = 0; d < 5; d++) {
      if (hadir[d]) {
        html += "<td>✔</td>";
        total++;
      } else {
        html += "<td>-</td>";
      }
    }

    html += "<td>" + String(total) + "</td></tr>";
  }
}
html += "</table>";
html += "</div>";

// Fungsi untuk clear data
html += "<hr>";
html += "<h3>System Control</h3>";
html += "<hr>";
html += "<h3>System Control</h3>";
html += "<div style='text-align:center;margin-top:20px'>";
html += "<button style='margin:6px;padding:8px 14px;background:#ff9800;color:white;border:none;border-radius:6px'";
html += " onclick=\"if(confirm('Clear data absensi SISWA?')) fetch('/clear_siswa').then(()=>location.reload());\">";
html += "Clear Data Siswa</button>";
html += "<button style='margin:6px;padding:8px 14px;background:#2196F3;color:white;border:none;border-radius:6px'";
html += " onclick=\"if(confirm('Clear data absensi GURU?')) fetch('/clear_guru').then(()=>location.reload());\">";
html += "Clear Data Guru</button>";
html += "</div>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleClearSiswa() {
  for (int i = 0; i < 100; i++) {
    if (logs[i].role == "siswa") {
      logs[i] = {};
    }
  }
  server.send(200, "text/plain", "Data siswa cleared");
}

void handleClearGuru() {
  for (int i = 0; i < 100; i++) {
    if (logs[i].role != "siswa") {
      logs[i] = {};
    }
  }
  server.send(200, "text/plain", "Data guru cleared");
}

// Fungsi untuk tambah user
void handleAddUser() {
  if (server.method() == HTTP_POST) {
    String uid = server.arg("uid");
    String name = server.arg("name");
    String role = server.arg("role");
    users[numUsers].uid = uid;
    users[numUsers].name = name;
    users[numUsers].role = role;
    numUsers++;
    Serial.println("User added: " + name);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Fungsi untuk edit user (baru)
void handleEditUser() {
  int index = server.arg("index").toInt();
  if (server.method() == HTTP_POST) {
    users[index].uid = server.arg("uid");
    users[index].name = server.arg("name");
    users[index].role = server.arg("role");
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }
  String html = "<h2>Edit User</h2><form action='/edituser' method='POST'>";
  html += "<input type='hidden' name='index' value='" + String(index) + "'>";
  html += "UID: <input type='text' name='uid' value='" + users[index].uid + "'><br>";
  html += "Name: <input type='text' name='name' value='" + users[index].name + "'><br>";
  html += "Role: <select name='role'><option value='" + users[index].role + "'>" + users[index].role + "</option>...</select><br>";  // Sederhana
  html += "<input type='submit' value='Update'></form>";
  server.send(200, "text/html", html);
}

// Fungsi untuk delete user (baru)
void handleDeleteUser() {
  int index = server.arg("index").toInt();
  for (int i = index; i < numUsers - 1; i++) {
    users[i] = users[i+1];
  }
  numUsers--;
  server.sendHeader("Location", "/");
  server.send(303);
}
