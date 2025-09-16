Natürlich, Werner! Hier ist dein vollständiges Git-Cheat-Sheet als **Markdown-Datei**, perfekt formatiert für dein Repository. Du kannst es als `GIT_CHEATSHEET.md` speichern:

---

```markdown
# 🧠 Git Cheat-Sheet für ESP32-Projekte

Ein kompakter Überblick über die wichtigsten Git-Befehle für deine ESP32-Entwicklung mit PlatformIO und GitHub.

---

## 🔧 Projekt initialisieren

```bash
git init                     # Git im aktuellen Projektordner starten
git add .                    # Alle Dateien zum Commit vormerken
git commit -m "Initial commit"  # Ersten Commit erstellen
```

---

## 🌐 Mit GitHub verbinden

```bash
git remote add origin https://github.com/dein-benutzername/dein-repo.git
git branch -M main          # Lokalen Branch auf "main" setzen
git push -u origin main     # Erstes Hochladen zu GitHub
```

---

## 📥 Änderungen holen (z. B. README auf GitHub erstellt)

```bash
git pull --rebase origin main  # Remote-Änderungen holen und lokal einfügen
```

---

## 📤 Änderungen hochladen

```bash
git add .                    # Änderungen vormerken
git commit -m "Neues Feature: WebSocket integriert"
git push                     # Änderungen zu GitHub hochladen
```

---

## 🕵️ Status & Verlauf prüfen

```bash
git status                   # Zeigt aktuelle Änderungen
git log --oneline            # Zeigt Commit-Historie kompakt
```

---

## 🧹 Dateien ignorieren

In `.gitignore` eintragen:

```
.pio/
.vscode/
data/
*.bin
```

---

## 🛑 Änderungen rückgängig machen

```bash
git restore datei.cpp        # Letzte Änderung an Datei verwerfen
git reset --hard HEAD        # Alle lokalen Änderungen verwerfen (vorsichtig!)
```

---

## 🧪 Branches (optional)

```bash
git checkout -b feature-xyz  # Neuen Branch erstellen
git switch main              # Zurück zum Haupt-Branch
```

---

## 🧼 Konflikte lösen

Wenn `git pull` Konflikte meldet:

- Öffne betroffene Datei
- Bearbeite die Konfliktstellen (`<<<<<<<`, `=======`, `>>>>>>>`)
- Danach:

```bash
git add .
git rebase --continue
```

---

## 🔐 GitHub Token (falls Passwort nicht mehr geht)

GitHub verlangt seit 2021 ein **Personal Access Token** statt Passwort:

- Erstellen unter: [github.com/settings/tokens](https://github.com/settings/tokens)
- Beim Push als Passwort eingeben

---

📦 Du kannst diese Datei direkt in dein Repository legen – sie ist ideal für dich und andere Entwickler, die dein Projekt nutzen oder erweitern möchten.
