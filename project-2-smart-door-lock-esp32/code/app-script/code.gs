function doPost(e) {
  var data = JSON.parse(e.postData.contents);
  var ss = SpreadsheetApp.getActiveSpreadsheet();

  var sheetSiswa = ss.getSheetByName('Absensi Siswa');
  var sheetGuru = ss.getSheetByName('Jurnal Guru');
  var sheetDenied = ss.getSheetByName('Log Denied');

  var timestamp = new Date().toLocaleString();
  var today = new Date();
  var tanggal = today.getDate();      // 1–31
  var hari = today.getDay();           // 0–6 (Sun–Sat)

  if (data.status === 'granted') {

    // ===================== SISWA =====================
    if (data.role === 'siswa') {
      var uidCol = 3; // Kolom C
      var startRow = 4;
      var lastRow = sheetSiswa.getLastRow();
      var row = -1;

      if (lastRow >= startRow) {
        var uids = sheetSiswa.getRange(startRow, uidCol, lastRow - startRow + 1, 1).getValues();
        for (var i = 0; i < uids.length; i++) {
          if (uids[i][0] == data.uid) {
            row = startRow + i;
            break;
          }
        }
      }

      // Jika UID belum ada → buat baris baru
      if (row === -1) {
        row = lastRow + 1;
        if (row < startRow) row = startRow;
        sheetSiswa.getRange(row, 2).setValue(data.name); // Nama
        sheetSiswa.getRange(row, 3).setValue(data.uid);  // UID
      }

      // Isi kolom tanggal
      if (tanggal >= 1 && tanggal <= 31) {
        sheetSiswa.getRange(row, tanggal + 3).setValue('H'); // D = tanggal 1
      }
    }

    // ===================== GURU =====================
    else if (data.role === 'guru_mapel') {
      var uidCol = 3;
      var startRow = 4;
      var lastRow = sheetGuru.getLastRow();
      var row = -1;

      if (lastRow >= startRow) {
        var uids = sheetGuru.getRange(startRow, uidCol, lastRow - startRow + 1, 1).getValues();
        for (var i = 0; i < uids.length; i++) {
          if (uids[i][0] == data.uid) {
            row = startRow + i;
            break;
          }
        }
      }

      // Jika UID belum ada → buat baris baru
      if (row === -1) {
        row = lastRow + 1;
        if (row < startRow) row = startRow;
        sheetGuru.getRange(row, 2).setValue(data.name);
        sheetGuru.getRange(row, 3).setValue(data.uid);
      }

      // Senin–Jumat saja
      if (hari >= 1 && hari <= 5) {
        sheetGuru.getRange(row, hari + 3).setValue('H'); // D = Senin
      }
    }
  }

  // ===================== DENIED =====================
  else if (data.status === 'denied') {
    var row = sheetDenied.getLastRow() + 1;
    if (row < 4) row = 4;
    sheetDenied.getRange(row, 2).setValue(data.name || "Unknown");
    sheetDenied.getRange(row, 3).setValue(data.uid);
    sheetDenied.getRange(row, 4).setValue(timestamp);
    sheetDenied.getRange(row, 5).setValue("denied");
  }

  return ContentService.createTextOutput("OK");
}

