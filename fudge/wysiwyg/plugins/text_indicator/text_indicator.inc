<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: text_indicator.inc,v 1.10 2006/12/06 05:11:11 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Text Indicator
*
* Purpose
*     A WYSIWYG plugin to show a sample character in the
*     font style/colours etc of the selected text.
*     Pure decoration - no real functionality.
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package Fudge
* @subpackage wysiwyg
*/
class text_indicator extends wysiwyg_plugin
{

    /**
    * Constructor
    * Adds a blank button which will become the text indicator
    *
    * @returns object Insert_Link
    * @access  public
    */
    function text_indicator() {
        $this->_add_button('textindicator','','','','');
    }


    /**
    * When the toolbar is updated, change the font style/colour etc of the indicator
    *
    * @returns void
    * @access  public
    */
    function print_plugin_update_toolbar() {
        ?>
        case "textindicator":
            if (!text) {
                try {
                    with (btn.element.style) {
                        backgroundColor = HTMLArea._makeColor(doc.queryCommandValue("backcolor"));
                        color = HTMLArea._makeColor(doc.queryCommandValue("forecolor"));
                        fontFamily = doc.queryCommandValue("fontname");
                        fontWeight = doc.queryCommandState("bold") ? "bold" : "normal";
                        fontStyle = doc.queryCommandState("italic") ? "italic" : "normal";
                    }
                } catch (e) {
                    with (btn.element.style) {
                        backgroundColor = "#FFFFFF";
                        color = "#000000";
                    }
                }
            }
        break;
        <?php
    }


    /**
    * Prints the text indicator on the toolbar instead of standard button
    *
    * @returns void
    * @access  public
    */
    function print_plugin_button_type() {
        ?>
        case "textindicator":
            el = document.createElement("div");
            el.appendChild(document.createTextNode("A"));
            el.className = "htmlarea-indicator";
            el.title = HTMLArea.I18N.tooltips.textindicator;
            var obj = {
                name: txt,     // the button name (i.e. 'bold')
                element: el,   // the UI element (DIV)
                enabled: true, // is it enabled?
                active: false, // is it pressed?
                text: false,   // enabled in text mode?
                cmd: "textindicator", // the command ID
                state: setButtonStatus // for changing state
            };
            tb_objects[txt] = obj;
        break;
        <?php
    }

}

?>