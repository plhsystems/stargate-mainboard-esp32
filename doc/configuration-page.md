# Configuration Page

> **⚠️ IMPLEMENTATION STATUS: PARTIALLY IMPLEMENTED (VIEW-ONLY)**
>
> **Current implementation** (`/setup/settings.html`):
> - ✅ View settings as table (key, description, type, min, default, max, value)
> - ✅ Fetch settings from `/api/settingsjson`
> - ❌ Edit functionality NOT implemented
> - ❌ Import functionality NOT implemented
> - ❌ Export functionality NOT implemented (can use API directly)
>
> **What works**:
> - Display all settings in read-only table format
> - Shows setting metadata (type, range, default)
>
> **What doesn't work**:
> - Inline editing of values
> - JSON file import/export via UI
> - Apply/save button
>
> Users can use the API endpoint `/api/settingsjson` directly for export.

## Intended Functionality

Configurations could be imported or exported into JSON.

## JSON file viewer

The user should be able to view or directly edit the JSON file.
It's mostly done for backup or to change expert level configurations.

**Note**: This is the planned functionality, not the current implementation.

