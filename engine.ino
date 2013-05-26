volatile byte temperature_target = 0;

static void set_temperature_target(byte target) {
  temperature_target = target;
  check_temperature();
}

static byte get_temperature_target() {
  return temperature_target;
}

static void check_temperature() {
  flame(get_temperature_target() > get_temperature());
}

static byte resume_file_id() {
  microfsfile resumefile = fs.open(1);
  if (!resumefile.is_valid() || resumefile.get_size() != 3) {
    fs.remove(1);
    return 0;
  }
  byte file_id = resumefile.read_byte(0);
  if (fs.open(file_id).is_valid()) {
    return file_id;
  }
  return 0;
}

static int resume_seconds() {
  microfsfile resumefile = fs.open(1);
  if (!resumefile.is_valid() || resumefile.get_size() != 3) {
    fs.remove(1);
    return 0; // FIXME
  }
  byte buf[2] = {0};
  if (resumefile.read_bytes(1, buf, sizeof(buf)) == sizeof(buf))
    return *(size_t*)(buf+1);
  return 0; // FIXME
}

static void resume_save(byte file_id, int seconds) {
  Serial.println(F("resume_save"));
  Serial.println(file_id);
  Serial.println(seconds);
  fs.remove(1);
  microfsfile resumefile = fs.create(3, 1);
  if (!resumefile.is_valid() || resumefile.get_size() != 3 || resumefile.get_id() != 1) {
    fs.remove(1);
    Serial.println(F("failed to save resume file"));
  }
  byte buf[3] = {0};
  buf[0] = file_id;
  *(size_t*)(buf+1) = seconds;
  if (resumefile.write_bytes(0, buf, sizeof(buf)) != sizeof(buf)) {
    fs.remove(1);
    Serial.println(F("failed to save resume file"));
  }
}

static void resume() {
  byte file_id = resume_file_id();
  if (resume_file_id() != 0) {
    Serial.println(F("resume"));
    Serial.println(file_id);
    uxmgr::get().show<main_menu>();
    uxmgr::get().next<program_progress>(file_id);
  }
}
