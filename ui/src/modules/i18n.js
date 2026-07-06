/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
import { createI18n } from 'vue-i18n'
import en from '@/locales/en.json'
import sv from '@/locales/sv.json'
import de from '@/locales/de.json'
import fr from '@/locales/fr.json'
import es from '@/locales/es.json'
import zh from '@/locales/zh.json'
import pl from '@/locales/pl.json'
import no from '@/locales/no.json'
import da from '@/locales/da.json'
import nl from '@/locales/nl.json'
import pt from '@/locales/pt.json'
import it from '@/locales/it.json'

const i18n = createI18n({
  legacy: false,
  locale: 'en',
  fallbackLocale: 'en',
  messages: { en, sv, de, fr, es, zh, pl, no, da, nl, pt, it }
})

export default i18n
