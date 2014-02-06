<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: special_chars.inc,v 1.3 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once dirname(__FILE__).'/../../../../core/include/init.inc';

/**
* WYSIWYG Plugin - Matrix Insert Link
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert and remove
*     a link in the editable document using an asset from MySoure Matrix
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class special_chars extends wysiwyg_plugin
{


	/**
	* Constructor
	* Adds the special character button to the toolbar
	*
	* @returns object special_chars
	* @access  public
	*/
	function special_chars(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('insertspecialchar','InsertSpecialChar', 'Insert Special Character','false','2');

	}//end constructor


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @returns void
	* @access  public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>

		<script type="text/javascript" language="Javascript">

			// Called when the user clicks on "Insert Special Character" button
			HTMLArea.prototype._insertSpecialChar = function() {
				var editor = this; // for nested functions
				var sel = this._getSelection();
				var range = this._createRange(sel);

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				var Width = 300;
				var Height = 390;
				editor_<?php echo $this->wysiwyg->name?>._object = this;
				var strPage = "<?php echo $this->get_popup_href('char_map.php')?>?width=" + Width + '&height=' + Height;
				this._popupDialog("insertSpecialChar", strPage, Width, Height, true, function(param) {
					if (param) {
						var html = param['entity'];
						html = html.toString();
						editor.insertHTML(html, range);

					}// end if param exists

				}, null);

			};

			<?php
			$entities = Array(
							'&Agrave;',
							'&agrave;',
							'&Aacute;',
							'&aacute;',
							'&Acirc;',
							'&acirc;',
							'&Atilde;',
							'&atilde;',
							'&Auml;',
							'&auml;',
							'&Aring;',
							'&aring;',
							'&AElig;',
							'&aelig;',
							'&Ccedil;',
							'&ccedil;',
							'&ETH;',
							'&eth;',
							'&Egrave;',
							'&egrave;',
							'&Eacute;',
							'&eacute;',
							'&Ecirc;',
							'&ecirc;',
							'&Euml;',
							'&euml;',
							'&Igrave;',
							'&igrave;',
							'&Iacute;',
							'&iacute',
							'&Icirc;',
							'&icirc;',
							'&Iuml;',
							'&iuml;',
							'&micro;',
							'&Ntilde;',
							'&ntilde;',
							'&Ograve;',
							'&ograve;',
							'&Oacute;',
							'&oacute;',
							'&Ocirc;',
							'&ocirc;',
							'&Otilde;',
							'&otilde;',
							'&Ouml;',
							'&ouml;',
							'&Oslash;',
							'&oslash;',
							'&szlig;',
							'&THORN;',
							'&thorn;',
							'&Ugrave;',
							'&ugrave;',
							'&Uacute;',
							'&uacute;',
							'&Ucirc;',
							'&ucirc;',
							'&Uuml;',
							'&uuml;',
							'&Yacute;',
							'&yacute;',
							'&yuml;',
							'&uml;',
							'&macr;',
							'&acute;',
							'&cedil;',
							'&iexcl;',
							'&iquest;',
							'&middot;',
							'&brvbar;',
							'&laquo;',
							'&raquo;',
							'&para;',
							'&sect;',
							'&copy;',
							'&reg;',
							'&sup1;',
							'&sup2;',
							'&sup3;',
							'&times;',
							'&divide;',
							'&frac14;',
							'&frac12;',
							'&frac34;',
							'&ordf;',
							'&ordm;',
							'&not;',
							'&deg;',
							'&plusmn;',
							'&curren;',
							'&cent;',
							'&pound;',
							'&yen;',
							'&Delta;',
							'&fnof;',
							'&Omega;',
							'&OElig;',
							'&oelig;',
							'&Scaron;',
							'&scaron;',
							'&Yuml;',
							'&circ;',
							'&tilde;',
							'&ndash;',
							'&mdash;',
							'&dagger;',
							'&Dagger;',
							'&bull;',
							'&hellip;',
							'&lsquo;',
							'&rsquo;',
							'&ldquo;',
							'&rdquo;',
							'&lsaquo;',
							'&rsaquo;',
							'&trade;',
							'&radic;',
							'&infin;',
							'&int;',
							'&part;',
							'&ne;',
							'&le;',
							'&ge;',
							'&sum;',
							'&permil;',
							'&prod;',
							'&pi;',
							'&loz;',
							'&shy;',
						);?>

			HTMLArea.prototype.entities = Array(
				<?php
				foreach ($entities as $val) {
					echo '\''.$val.'\',';
				}
				?>
				'&dummycharactercode');
		</script>
		<?php

	}//end paint_generic()


	/**
	* Prints shortcuts for the buttons
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{

	}//end print_plugin_shortcuts()


	/**
	* Call the insert image function when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "insertspecialchar":
			this._insertSpecialChar();
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
