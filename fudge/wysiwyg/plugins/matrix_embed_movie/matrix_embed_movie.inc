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
* $Id: matrix_embed_movie.inc,v 1.33 2013/07/25 23:25:16 lwright Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Matrix Embed Movie
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert a movie
*     in the editable document using a file asset from MySoure Matrix
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_embed_movie extends wysiwyg_plugin
{

	var $_img_url = '';


	/**
	* Constructor
	* Adds the insert image button to this plugin
	*
	* @access public
	*/
	function matrix_embed_movie(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixembedmovie','MatrixEmbedMovie','Embed Movie','false','53');
		$this->_img_url = $wysiwyg->web_path.'plugins/'.$this->get_popup_href('embedded_object.gif');

	}//end constructor


	/**
	* Prints a generic embed movie function that can be used by any editor
	*
	* @returns void
	* @access  public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript" language="Javascript">
			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._matrixEmbedMovie = function() {
				var editor = this; // for nested functions

				var sel = this._getSelection();
				var range = this._createRange(sel);

				var style          = "";
				var width          = "";
				var height         = "";
				var fileid         = "0";
				var auto_start     = "0";
				var show_controls  = "0";
				var embed_loop     = "0";
				var is_file        = false;
				var fileurl        = '';
				var fileprotocol   = '';

				// loop through and try and find a selected embed tag
				if (HTMLArea.is_gecko) {
					var fragment = this._doc.createDocumentFragment();
					var div = this._doc.createElement("div");
					div.innerHTML = this.getSelectedHTML();
					while (div.firstChild) {
						if (div.firstChild.tagName == "IMG" && div.firstChild.getAttribute('sq_embeded') == "1") {
							is_file = true;
							var embed_tag = div.firstChild;
							break;
						}
						// the following call also removes the node from div
						fragment.appendChild(div.firstChild);
					}
				} else if (HTMLArea.is_ie) {
					if (range.length) {
						if (range.item(0).tagName == "IMG" && range.item(0).getAttribute('sq_embeded') == "1") {
							is_file = true;
							var embed_tag = range.item(0);
						}
					}
				}

				// if there is an existing image, get its properties
				if (is_file && embed_tag) {
					fileid        = embed_tag.getAttribute('fileid');
					if (fileid != null) {
						fileurl = './?a=' + fileid;
					} else {
						fileparts = embed_tag.getAttribute('fileurl');
						if (fileparts != null) {
							if (fileparts.substring(0, 7) == 'http://') {
								fileprotocol = 'http://';
								fileurl = fileparts.substring(7);
							} else if (fileparts.substring(0, 8) == 'https://') {
								fileprotocol = 'https://';
								fileurl = fileparts.substring(8);
							} else if (fileparts.substring(0, 6) == 'ftp://') {
								fileprotocol = 'ftp://';
								fileurl = fileparts.substring(6);
							}
						}
					}
					auto_start    = embed_tag.getAttribute('auto_start');
					show_controls = embed_tag.getAttribute('show_controls');
					embed_loop    = embed_tag.getAttribute('embed_loop');

					// try getting the width and height from the style
					style = embed_tag.style;
					width  = style.width;
					height = style.height;

					// if that fails, lets try getting them from the tag
					if (!width)  { width  = embed_tag.getAttribute('width');  }
					if (!height) { height = embed_tag.getAttribute('height'); }

					// make sure they are numbers
					width  = parseInt(width);
					height = parseInt(height);

					// failsafe
					if (!width)  { width  = "" }
					if (!height) { height = "" }

				}

				var in_popup = '0';
				if (this._inPopup == true) { in_popup = '1'; }

				editor_<?php echo $this->wysiwyg->name?>._object = this;
				strPage = "<?php echo $this->get_popup_href('embed_movie_frames.php')?>?f_fileurl=" + escape(fileurl) + "&f_fileprotocol=" + escape(fileprotocol) + "&f_width=" + escape(width) + "&f_height=" + escape(height) + "&f_auto_start=" + escape(auto_start) + "&f_show_controls=" + escape(show_controls) + "&f_embed_loop=" + escape(embed_loop) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				this._popupDialog("matrixEmbedMovie", strPage, 780, 500, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}
					var html = '<img sq_embeded="1" src="<?php echo $this->_img_url; ?>"';
					html += ' style="width: ' + param["f_width"] + 'px; height: ' + param["f_height"] + 'px; border: 2px dashed black;"';
					html += ' width="'         + param["f_width"] + '"';
					html += ' height="'        + param["f_height"] + '"';
					if (param["use_external"] == true) {
						html += ' fileurl="'	+ param["external_url"] + '"';
					} else {
						html += ' fileid="'		+ param["f_fileid"] + '"';
					}
					html += ' auto_start="'    + param["f_auto_start"] + '"';
					html += ' show_controls="' + param["f_show_controls"] + '"';
					html += ' embed_loop="'    + param["f_embed_loop"] + '"';
					html += '>';

					if (!is_file || HTMLArea.is_gecko) {
						// replace text with new image
						editor.insertHTML(html, range);
					} else {
						// we are replacing an existing image in IE
						embed_tag.outerHTML = html;
					}

				}, null);
			};
		</script>
		<?php

	}//end paint_generic()


	/**
	* Call the embed movie function when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "matrixembedmovie":
			this._matrixEmbedMovie();
			return;
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Replaces plugin specific HTML that was generated with proper HTML for emebedding
	*
	* @param string	&html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$matches = Array();
		preg_match_all('|(<img[^\>]+sq_embeded="1"([^\>]+)?>)|i', $html, $matches);

		foreach ($matches[1] as $match) {
			$old_html = $match;

			$match = strtolower($match);
			$match = substr($match, 4);
			$match = substr($match, 0, -1);
			$match = str_replace('src="'.$this->_img_url.'"', '', $match);

			$pieces = Array();
			preg_match_all('|\s+([^=]+)="([^"]+)"|', $match, $pieces);

			$movie_vars = Array();
			for (reset($pieces[1]); NULL !== ($k = key($pieces[1])); next($pieces[1])) {
				$var_name  = trim($pieces[1][$k]);
				$var_value = trim($pieces[2][$k]);
				$movie_vars[$var_name] = $var_value;
			}
			$movie_url = '';
			if (isset($movie_vars['fileurl'])) {
				$filename = $movie_vars['fileurl'];
				$movie_url = $movie_vars['fileurl'];
				$movie_url = $filename;
			} else {
				if (!isset($movie_vars['fileid'])) continue;
				$fileid = $movie_vars['fileid'];
				$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($fileid);
				
				if (is_null($file)) continue;
				if (!($file instanceof File)) {
					trigger_localised_error('SYS0089', E_USER_WARNING, $file->id, 'File');
					continue;
				}

				$filename = $file->name;
				$movie_url = './?a='.$fileid;
			}
			// first try getting width and height from the style
			$width  = '';
			$height = '';
			if (isset($movie_vars['style'])) {
				$style = $movie_vars['style'];
				$style_pieces = Array();
				preg_match_all('|([^\s:]+):\s([^\s;]+);?|i', $style, $style_pieces);

				$style_vars = Array();
				for (reset($style_pieces[1]); NULL !== ($k = key($style_pieces[1])); next($style_pieces[1])) {
					$var_name  = trim($style_pieces[1][$k]);
					$var_value = trim($style_pieces[2][$k]);
					$style_vars[$var_name] = $var_value;
				}

				if (isset($style_vars['width'])) {
					$width = 'width="'.str_replace('px','',$style_vars['width']).'"';
				}
				if (isset($style_vars['height'])) {
					$height = 'height="'.str_replace('px','',$style_vars['height']).'"';
				}
			}

			// if we didnt get width and height from the style, try the tag
			if (empty($width)) {
				if (isset($movie_vars['width'])) {
					$width = 'width="'.str_replace('px','',$movie_vars['width']).'"';
				}
			}
			if (empty($height)) {
				if (isset($movie_vars['height'])) {
					$height = 'height="'.str_replace('px','',$movie_vars['height']).'"';
				}
			}

			ob_start();

			?>
			<sq_wysiwyg_embed_movie <?php echo $match; ?>>
			<?php
			switch (strtolower(get_file_type($filename))) {

				// Quicktime Movie
				case 'mov' :
					if (!isset($movie_vars['embed_loop'])) {
						$movie_vars['embed_loop']    = '0';
					}
					if (!isset($movie_vars['show_controls'])) {
						$movie_vars['show_controls'] = '0';
					}

					?><embed src="<?php echo $movie_url; ?>" controller="<?php echo ($movie_vars['show_controls']) ? 'true' : 'false'?>" cache="true" loop="<?php echo ($movie_vars['embed_loop']) ? 'true' : 'false'; ?>" <?php echo $width; ?> <?php echo $height; ?>><?php
				break;

				// Shockwave Flash
				case 'swf' :
					?><object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=4,0,2,0" <?php echo $width; ?> <?php echo $height; ?>>
						<param name="movie"   value="<?php echo $movie_url; ?>">
						<param name="quality" value="high">
						<param name="scale"   value="exactfit">
						<embed src="<?php echo $movie_url; ?>" quality="high" pluginspage="http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash" type="application/x-shockwave-flash" <?php echo $width; ?> <?php echo $height; ?> scale="exactfit"></embed>
					</object><?php
				break;

				// Windows Media File
				case 'wmv' :
				case 'asf' :
				case 'asx' :
				case 'mpeg' :
				case 'mpg' :
					if (!isset($movie_vars['embed_loop'])) {
						$movie_vars['embed_loop']    = '0';
					}
					if (!isset($movie_vars['show_controls'])) {
						$movie_vars['show_controls'] = '0';
					}
					if (!isset($movie_vars['auto_start'])) {
						$movie_vars['auto_start']    = '0';
					}
					?><object id="WinMedia" classid="CLSID:22d6f312-b0f6-11d0-94ab-0080c74c7e95"
					  codebase= "http://activex.microsoft.com/activex/controls/mplayer/en/nsmp2inf.cab#Version=5,1,52,701"
					  <?php echo $width; ?> <?php echo $height; ?>
					  standby="Loading Microsoft Windows Media Player components..." type="application/x-oleobject">
						<param name="FileName"     value="<?php echo $movie_url; ?>">
						<param name="AutoStart"    value="<?php echo ($movie_vars['auto_start'])    ? 'true' : 'false'; ?>">
						<param name="ShowControls" value="<?php echo ($movie_vars['show_controls']) ? 'true' : 'false'; ?>">
						<param name="loop"         value="<?php echo ($movie_vars['embed_loop'])    ? 'true' : 'false';?>">
						<embed type="application/x-mplayer2"
							pluginspage="http://www.microsoft.com/Windows/MediaPlayer/"
							src="<?php echo $movie_url; ?>"
							name="MediaPlayer"
							AutoStart="<?php echo ($movie_vars['auto_start'])       ? 'true' : 'false'; ?>"
							ShowControls="<?php echo ($movie_vars['show_controls']) ? 'true' : 'false'; ?>"
							loop="<?php echo ($movie_vars['embed_loop'])            ? 'true' : 'false'; ?>"
							<?php echo $width; ?>
							<?php echo $height; ?>
						></embed>
					</object><?php
				break;

			}//end switch

			?></sq_wysiwyg_embed_movie><?php
			$new_html = ob_get_contents();
			ob_end_clean();

			$html = str_replace($old_html, $new_html, $html);

		}//end foreach $matches

		// make sure the htmlTidy plugin knows about our special tag
		if ($this->wysiwyg->plugin_loaded('html_tidy')) {
			$html_tidy =& $this->wysiwyg->get_plugin('html_tidy');
			$html_tidy->new_inline_tag('sq_wysiwyg_embed_movie');
		}

	}//end process()


	/**
	* Sets the initial contents of the WYSIWYG editor
	*
	* @param string	&$contents	Some HTML content
	*
	* @return void
	* @access public
	*/
	function set_contents(&$contents)
	{
		// replace the special embedded tags with standard img tags
		$contents = preg_replace('|(<sq_wysiwyg_embed_movie([^>]*)?>)(.*?)(</sq_wysiwyg_embed_movie>)|si', '<IMG src="'.$this->_img_url.'" $2>', $contents);

	}//end set_contents()


}//end class

?>
