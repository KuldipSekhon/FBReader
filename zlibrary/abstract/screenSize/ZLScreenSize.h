/*
 * Copyright (C) 2005 Nikolay Pultsin <geometer@mawhrin.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ZLSCREENSIZE_H__
#define __ZLSCREENSIZE_H__

class ZLScreenSize {

public:
	enum Size {
		SIZE_DESKTOP,
		SIZE_240x320,
		SIZE_640x480
	};

	static Size getSize() { return ourInstance->getSizeInternal(); }
	static void deleteInstance() { delete ourInstance; }

protected:
	static ZLScreenSize *ourInstance;

	ZLScreenSize() {}
	virtual ~ZLScreenSize() {}
	virtual Size getSizeInternal() const = 0;
};

#endif /* __ZLSCREENSIZE_H__ */
