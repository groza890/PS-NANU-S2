#pragma once
#include <ESP_Mail_Client.h>

// Obiecte globale pentru sesiune SMTP
SMTPSession     smtp;      // transport SMTP
ESP_Mail_Session session;  // configurare server + user/pass
SMTP_Message     message;  // structura email-ului

namespace Mailer {

  // Trebuie să existe această funcție!
  inline void begin() {
    // 1) Setup server SMTP
    session.server.host_name = "smtp.gmail.com";
    session.server.port      = 465;
    session.login.email      = "catalingroza89@gmail.com";      // -> înlocuiește
    session.login.password   = "dgno bwua dtzu tzau"; // -> înlocuiește
    session.login.user_domain= "";

    // 2) Permite reconectarea automată la Wi-Fi
    MailClient.networkReconnect(true);

    // 3) Debug minimal
    smtp.debug(1);

    // 4) Deschide conexiunea SMTP
    if (!smtp.connect(&session)) {
      Serial.printf("SMTP connect failed: %s\n", smtp.errorReason().c_str());
    } else {
      Serial.println("✅ SMTP connected");
    }
  }

  // Și această funcție pentru trimiterea mail-ului
  inline void sendFloodAlert(const String &msg) {
  // 1) Conectare SMTP fresh
  if (!smtp.connect(&session)) {
    Serial.printf("SMTP connect failed: %s\n", smtp.errorReason().c_str());
    return;
  }

  // 2) Creează un mail nou
  SMTP_Message mail;
  mail.sender.name    = "ESP32 Flood Alert";
  mail.sender.email   = session.login.email;
  mail.subject        = "⚠️ ALERTĂ INUNDAȚIE!";
  mail.text.charSet   = "utf-8";
  mail.text.content   = msg;
  mail.addRecipient("Destinatar", "corneliu.groza@student.upt.ro");

  // 3) Trimite
  if (!MailClient.sendMail(&smtp, &mail)) {
    Serial.printf("Mail send failed: %s\n", smtp.errorReason().c_str());
  } else {
    Serial.println("✉️ Mail trimis cu succes");
  }

  // 4) Închide sesiunea
  smtp.closeSession();
}


} // namespace Mailer
