/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: lister.js,v 1.1.1.1 2004/10/13 01:31:48 arailean Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Lister class. Constructor
*
* @param	FromID	ID of the list object, contained source
* @param	ToID	ID of the list object - destination
* @param	HideID	ID of the hidden list object, which will contain all selected items
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @return null
* @access public
*/
function Lister(FromID, ToID, HideID)
{
	this.fromID	  = FromID;
	this.toID 	  = ToID;
	this.hideID	  = HideID;
	this.From	  = document.getElementById(FromID);
	this.To		  = document.getElementById(ToID);
	this.Hide	  = document.getElementById(HideID);
	this.ToOption = Array();

/**
* Moves selected item(s) from source to destination
*
* @return void
* @access public
*/
	this.From2To = function() {
		for (var i=0;i<this.From.length;i++)
		{
			if (this.From.options[i].selected) {
				var notmoved = true;
				for(var j = 0;j<this.ToOption.length;j++) if (this.ToOption[j][1] == i) notmoved = false;
				if (notmoved) {
					this.ToOption.push(Array(this.To.length, i));
					this.To.options[this.To.length] = new Option(this.From.options[i].text, this.From.options[i].value, this.From.options[i].defaultSelected, true);
					this.Hide.options[this.Hide.length] = new Option(this.From.options[i].text, this.From.options[i].value, true, true);
					this.From.options[i].text = document.getElementById("bull").value + this.From.options[i].text;
				}
			}
		}
		this.Clear();

	}//end From2To()


/**
* moves selected item(s) from destination to source
*
* @return void
* @access public
*/
	this.To2From = function() {
		for (var i=0;i<this.To.length;i++)
		{
			if (this.To.options[i].selected) {
				var len = document.getElementById("bull").value.length;
				for(var j = 0;j<this.ToOption.length;j++)
				{
					if (this.ToOption[j] != null && typeof this.ToOption[j] != "undefined") {
						if (this.ToOption[j][0] == i) {
							this.From.options[this.ToOption[j][1]].text = this.From.options[this.ToOption[j][1]].text.substring(len);
							this.ToOption.splice(j,1);
						}
						if (this.ToOption[j] != null && typeof this.ToOption[j] != "undefined") {
							if (this.ToOption[j][0] > i) this.ToOption[j][0]--;
						}
					}
				}
				this.To.options[i] = null;
				this.Hide.options[i] = null;
				i--;
			}
		}

	}//end To2From()


/**
* adds option to list
*
* @param	id			id of the new option
* @param	value		value of the option
* @param	selected	true if the option is selected
*
* @return void
* @access public
*/
	this.addOption = function(id, value, selected) {
		this.From.options[this.From.length] = new Option(value, id, false, (typeof selected == "undefined")?false:selected);

	}//end addOption()


/**
* moves all necessary options to source (runs at the begining after addOption calls)
*
* @return void
* @access public
*/
	this.process = function() {
		this.From2To();
		for (var i=0;i<this.To.length;i++) {
			this.To.options[i].selected = false;
		}
		for (var i=0;i<this.From.length;i++) {
			this.From.options[i].selected = false;
		}

	}//end process()


/**
* makes in source list only one item selected
*
* @return void
* @access public
*/
	this.Clear = function() {
		var firstSelected = -1;
		for (var i=0;i<this.To.length;i++) {
			if (firstSelected == -1 && this.To.options[i].selected) firstSelected = i;
			else
				if (this.To.options[i].selected) this.To.options[i].selected = false;
		}

	}//end Clear()

}


