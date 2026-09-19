// trial_record.h
// API untuk menulis/membaca file trial record (.lm_trial) dengan HMAC-SHA256.
// File disimpan di per-user AppData path, ditandatangani dengan secret
// yang di-embed dalam binary (constexpr byte array).
//
// Hakiki: file ini hanya tamper-evident, bukan delete-proof.
// Penghapusan file = sinyal fresh install (diterima).
#pragma once

#include "license_helper.h"

// namespace license sudah dideklarasikan di license_helper.h
// Struktur TrialRecord dan fungsi writeTrialRecord/readTrialRecord
// dideklarasikan di sana juga. Header ini hanya berisi dokumentasi
// tambahan dan dependensi implisit.
//
// Penggunaan:
//   auto path = license::writeTrialRecord("2026-09-19");
//   auto rec  = license::readTrialRecord();
//   if (rec && rec->valid) { ... }
//
// File: <AppDataLocation>/.lm_trial
// Format internal: line1 = install_date, line2 = HMAC-SHA256 hex (64 char)
