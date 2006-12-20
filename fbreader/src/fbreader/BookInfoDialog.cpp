/*
 * FBReader -- electronic book reader
 * Copyright (C) 2004-2006 Nikolay Pultsin <geometer@mawhrin.net>
 * Copyright (C) 2005 Mikhail Sobolev <mss@mawhrin.net>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <algorithm>

#include <ZLEncodingConverter.h>
#include <ZLDialogManager.h>
#include <ZLOptionsDialog.h>
#include <ZLOptionEntry.h>
#include <ZLFile.h>

#include <optionEntries/ZLStringInfoEntry.h>
#include <optionEntries/ZLSimpleOptionEntry.h>

#include "BookInfoDialog.h"

#include "../description/Author.h"
#include "../hyphenation/TeXHyphenator.h"

class AuthorSortKeyEntry : public ZLStringOptionEntry {

public:
	AuthorSortKeyEntry(BookInfoDialog &dialog);

	const std::string &name() const;
	const std::string &initialValue() const;
	void onAccept(const std::string &value);

private:
	BookInfoDialog &myInfoDialog;
};

class AuthorDisplayNameEntry : public ZLComboOptionEntry {

public:
	AuthorDisplayNameEntry(BookInfoDialog &dialog);

	const std::string &name() const;
	const std::string &initialValue() const;
	const std::vector<std::string> &values() const;
	void onAccept(const std::string &value);
	void onValueSelected(int index);

private:
	BookInfoDialog &myInfoDialog;
	mutable std::vector<std::string> myValues;
	AuthorPtr myCurrentAuthor;

friend class AuthorSortKeyEntry;
friend class SeriesTitleEntry;
};

class EncodingEntry : public ZLComboOptionEntry {

public:
	EncodingEntry(ZLStringOption &encodingOption);

	const std::string &name() const;
	const std::string &initialValue() const;
	const std::vector<std::string> &values() const;
	void onAccept(const std::string &value);

private:
	ZLStringOption &myEncodingOption;
};

class LanguageEntry : public ZLComboOptionEntry {

public:
	LanguageEntry(ZLStringOption &encodingOption);

	const std::string &name() const;
	const std::string &initialValue() const;
	const std::vector<std::string> &values() const;
	void onAccept(const std::string &value);

private:
	ZLStringOption &myLanguageOption;
};

class SeriesTitleEntry : public ZLComboOptionEntry {

public:
	SeriesTitleEntry(BookInfoDialog &dialog);

	const std::string &name() const;
	const std::string &initialValue() const;
	const std::vector<std::string> &values() const;
	void onAccept(const std::string &value);

	void onValueEdited(const std::string &value);
	void onValueSelected(int index);

private:
	BookInfoDialog &myInfoDialog;
	AuthorPtr myOriginalAuthor;
	mutable std::vector<std::string> myValues;
};

static std::vector<std::string> AUTO_ENCODING;

AuthorDisplayNameEntry::AuthorDisplayNameEntry(BookInfoDialog &dialog) : ZLComboOptionEntry(true), myInfoDialog(dialog) {
}

const std::string &AuthorDisplayNameEntry::name() const {
	static const std::string NAME = "Author (display name)";
	return NAME;
}

const std::string &AuthorDisplayNameEntry::initialValue() const {
	return myInfoDialog.myBookInfo.AuthorDisplayNameOption.value();
}

const std::vector<std::string> &AuthorDisplayNameEntry::values() const {
	if (myValues.empty()) {
		const std::string &initial = initialValue();
		bool addInitial = true;
		const std::vector<AuthorPtr> &authors = myInfoDialog.myCollection.authors();
		for (std::vector<AuthorPtr>::const_iterator it = authors.begin(); it != authors.end(); ++it) {
			const std::string name = (*it)->displayName();
			if (addInitial && (name == initial)) {
				addInitial = false;
			}
			myValues.push_back(name);
		}
		if (addInitial) {
			myValues.push_back(initial);
		}
	}
	return myValues;
}

void AuthorDisplayNameEntry::onAccept(const std::string &value) {
	myInfoDialog.myBookInfo.AuthorDisplayNameOption.setValue(value);
}

void AuthorDisplayNameEntry::onValueSelected(int index) {
	const std::vector<AuthorPtr> &authors = myInfoDialog.myCollection.authors();
	myCurrentAuthor = authors[index];
	myInfoDialog.myAuthorSortKeyEntry->resetView();
	myInfoDialog.mySeriesTitleEntry->resetView();
}

AuthorSortKeyEntry::AuthorSortKeyEntry(BookInfoDialog &dialog) : myInfoDialog(dialog) {
}

const std::string &AuthorSortKeyEntry::name() const {
	static const std::string NAME = "Author (sort key)";
	return NAME;
}

const std::string &AuthorSortKeyEntry::initialValue() const {
	AuthorPtr currentAuthor = myInfoDialog.myAuthorDisplayNameEntry->myCurrentAuthor;
	return currentAuthor.isNull() ?
		myInfoDialog.myBookInfo.AuthorSortKeyOption.value() :
		currentAuthor->sortKey();
}

void AuthorSortKeyEntry::onAccept(const std::string &value) {
	myInfoDialog.myBookInfo.AuthorSortKeyOption.setValue(value);
}

static const std::string AUTO = "auto";

EncodingEntry::EncodingEntry(ZLStringOption &encodingOption) : myEncodingOption(encodingOption) {
	setActive(initialValue() != AUTO);
}

const std::vector<std::string> &EncodingEntry::values() const {
	if (initialValue() == AUTO) {
		if (AUTO_ENCODING.empty()) {
			AUTO_ENCODING.push_back(AUTO);
		}
		return AUTO_ENCODING;
	}
	return ZLEncodingConverter::knownEncodings();
}

const std::string &EncodingEntry::name() const {
	static const std::string NAME = "Encoding";
	return NAME;
}

const std::string &EncodingEntry::initialValue() const {
	return myEncodingOption.value();
}

void EncodingEntry::onAccept(const std::string &value) {
	myEncodingOption.setValue(value);
}

LanguageEntry::LanguageEntry(ZLStringOption &encodingOption) : myLanguageOption(encodingOption) {
}

const std::string &LanguageEntry::initialValue() const {
	const std::vector<std::string> &codes = TeXHyphenator::languageCodes();
	const std::vector<std::string> &names = TeXHyphenator::languageNames();
	const size_t index = std::find(codes.begin(), codes.end(), myLanguageOption.value()) - codes.begin();
	return (index < names.size()) ? names[index] : names.back();
}

const std::vector<std::string> &LanguageEntry::values() const {
	return TeXHyphenator::languageNames();
}

const std::string &LanguageEntry::name() const {
	static const std::string NAME = "Language";
	return NAME;
}

void LanguageEntry::onAccept(const std::string &value) {
	const std::vector<std::string> &codes = TeXHyphenator::languageCodes();
	const std::vector<std::string> &names = TeXHyphenator::languageNames();
	const size_t index = std::find(names.begin(), names.end(), value) - names.begin();
	myLanguageOption.setValue((index < codes.size()) ? codes[index] : codes.back());
}

SeriesTitleEntry::SeriesTitleEntry(BookInfoDialog &dialog) : ZLComboOptionEntry(true), myInfoDialog(dialog) {
	const std::vector<AuthorPtr> &authors = myInfoDialog.myCollection.authors();
	const std::string &authorName = myInfoDialog.myBookInfo.AuthorDisplayNameOption.value();
	const std::string &authorKey = myInfoDialog.myBookInfo.AuthorSortKeyOption.value();
	for (std::vector<AuthorPtr>::const_iterator it = authors.begin(); it != authors.end(); ++it) {
		if ((authorName == (*it)->displayName()) &&
				(authorKey == (*it)->sortKey())) {
			myOriginalAuthor = *it;
			break;
		}
	}
}

const std::string &SeriesTitleEntry::initialValue() const {
	return myInfoDialog.myBookInfo.SequenceNameOption.value();
}

const std::string &SeriesTitleEntry::name() const {
	static const std::string NAME = "Series Title";
	return NAME;
}

const std::vector<std::string> &SeriesTitleEntry::values() const {
	myValues.clear();
	std::set<std::string> valuesSet;
	valuesSet.insert(initialValue());
	valuesSet.insert("");
	if (!myOriginalAuthor.isNull()) {
		const Books &books = myInfoDialog.myCollection.books(myOriginalAuthor);
		for (Books::const_iterator it = books.begin(); it != books.end(); ++it) {
			valuesSet.insert((*it)->sequenceName());
		}
	}
	AuthorPtr currentAuthor = myInfoDialog.myAuthorDisplayNameEntry->myCurrentAuthor;
	if (!currentAuthor.isNull() && (currentAuthor != myOriginalAuthor)) {
		const Books &books = myInfoDialog.myCollection.books(currentAuthor);
		for (Books::const_iterator it = books.begin(); it != books.end(); ++it) {
			valuesSet.insert((*it)->sequenceName());
		}
	}
	for (std::set<std::string>::const_iterator it = valuesSet.begin(); it != valuesSet.end(); ++it) {
		myValues.push_back(*it);
	}
	return myValues;
}

void SeriesTitleEntry::onAccept(const std::string &value) {
	myInfoDialog.myBookInfo.SequenceNameOption.setValue(value);
}

void SeriesTitleEntry::onValueSelected(int index) {
	myInfoDialog.myBookNumberEntry->setVisible(index != 0);
}

void SeriesTitleEntry::onValueEdited(const std::string &value) {
	myInfoDialog.myBookNumberEntry->setVisible(!value.empty());
}

BookInfoDialog::BookInfoDialog(const BookCollection &collection, const std::string &fileName) : myCollection(collection), myBookInfo(fileName) {
	myDialog = ZLDialogManager::instance().createOptionsDialog("InfoDialog", "FBReader - Book Info");

	myFileNameEntry = new ZLStringInfoEntry("File", fileName);
	myBookTitleEntry = new ZLSimpleStringOptionEntry("Title", myBookInfo.TitleOption);
	myAuthorDisplayNameEntry = new AuthorDisplayNameEntry(*this);
	myAuthorSortKeyEntry = new AuthorSortKeyEntry(*this);
	myEncodingEntry = new EncodingEntry(myBookInfo.EncodingOption);
	myLanguageEntry = new LanguageEntry(myBookInfo.LanguageOption);
	mySeriesTitleEntry = new SeriesTitleEntry(*this);
	myBookNumberEntry = new ZLSimpleSpinOptionEntry("Book Number", myBookInfo.NumberInSequenceOption, 1);
	mySeriesTitleEntry->onValueEdited(mySeriesTitleEntry->initialValue());

	ZLDialogContent &commonTab = myDialog->createTab("Common");
	commonTab.addOption(myFileNameEntry);
	commonTab.addOption(myBookTitleEntry);
	commonTab.addOption(myAuthorDisplayNameEntry);
	commonTab.addOption(myAuthorSortKeyEntry);
	commonTab.addOption(myEncodingEntry);
	commonTab.addOption(myLanguageEntry);

	ZLDialogContent &seriesTab = myDialog->createTab("Series");
	seriesTab.addOption(mySeriesTitleEntry);
	seriesTab.addOption(myBookNumberEntry);
	/*
	ZLOrderOptionEntry *orderEntry = new ZLOrderOptionEntry();
	orderEntry->values().push_back("First");
	orderEntry->values().push_back("Second");
	orderEntry->values().push_back("Third");
	orderEntry->values().push_back("Fourth");
	orderEntry->values().push_back("Fifth");
	sequenceTab.addOption(orderEntry);
	*/

	FormatPlugin *plugin = PluginCollection::instance().plugin(ZLFile(fileName), false);
	if (plugin != 0) {
		myFormatInfoPage = plugin->createInfoPage(*myDialog, fileName);
	}
}