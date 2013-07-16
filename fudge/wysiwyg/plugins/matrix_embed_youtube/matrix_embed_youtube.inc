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
* $Id: matrix_embed_youtube.inc,v 1.9.6.1 2013/07/16 02:06:14 lwright Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Matrix Embed YouTube
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert a YouTube movie
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.9.6.1 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class matrix_embed_youtube extends wysiwyg_plugin
{

	var $_img_url = '';


	/**
	* Constructor
	* Adds the insert image button to this plugin
	*
	* @access public
	*/
	function matrix_embed_youtube(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('matrixembedyoutube','MatrixEmbedYouTube','Embed YouTube','false','67');
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
			HTMLArea.prototype._matrixEmbedYouTube = function() {
				var editor = this; // for nested functions

				var sel = this._getSelection();
				var range = this._createRange(sel);

				var style          = "";
				var width          = "480";
				var height         = "385";
				var auto_start     = "0";
				var loop           = "0";
				var full_screen    = "1";
				var is_file        = false;
				var vid	           = "";
				var video_url      = "";
				var show_border    = "";
				var colour1        = "";
				var colour2        = "";
				var rel            = "";
				var egm            = "";
				var enable_js      = "";

				// loop through and try and find a selected embed tag
				if (HTMLArea.is_gecko) {
					var fragment = this._doc.createDocumentFragment();
					var div = this._doc.createElement("div");
					div.innerHTML = this.getSelectedHTML();
					while (div.firstChild) {
						if (div.firstChild.tagName == "IMG" && div.firstChild.getAttribute('sq_youtube_embeded') == "1") {
							is_file = true;
							var embed_tag = div.firstChild;
							break;
						}
						// the following call also removes the node from div
						fragment.appendChild(div.firstChild);
					}
				} else if (HTMLArea.is_ie) {
					if (range.length) {
						if (range.item(0).tagName == "IMG" && range.item(0).getAttribute('sq_youtube_embeded') == "1") {
							is_file = true;
							var embed_tag = range.item(0);
						}
					}
				}

				// if there is an existing image, get its properties
				if (is_file && embed_tag) {
					vid           = embed_tag.getAttribute('vid');
					video_url     = embed_tag.getAttribute('video_url');
					auto_start    = embed_tag.getAttribute('auto_start');
					loop          = embed_tag.getAttribute('loop');
					full_screen   = embed_tag.getAttribute('full_screen');
					show_border   = embed_tag.getAttribute('show_border');
					colour1       = embed_tag.getAttribute('colour1');
					colour2       = embed_tag.getAttribute('colour2');
					width         = embed_tag.getAttribute('width');
					height        = embed_tag.getAttribute('height');
					rel           = embed_tag.getAttribute('rel');
					egm           = embed_tag.getAttribute('egm');
					enable_js     = embed_tag.getAttribute('enable_js');

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
				strPage = "<?php echo $this->get_popup_href('embed_youtube_frames.php')?>?f_vid=" + escape(vid) + "&f_video_url=" + escape(video_url) + "&f_width=" + escape(width) + "&f_height=" + escape(height) + "&f_auto_start=" + escape(auto_start) + "&f_loop=" + escape(loop) + "&f_full_screen=" + escape(full_screen) + "&f_show_border=" + escape(show_border) + "&f_colour1=" + escape(colour1) + "&f_colour2=" + escape(colour2) + "&f_rel=" + escape(rel) + "&f_egm=" + escape(egm) + "&f_enable_js=" + escape(enable_js) + "&editor_name=" + escape("<?php echo $this->wysiwyg->name; ?>") + "&in_popup=" + escape(in_popup);

				var popup_height = 300;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_height += 15;

				this._popupDialog("matrixEmbedYouTube", strPage, 780, popup_height, true, function(param) {
					if (!param) {
						// user must have pressed Cancel
						return false;
					}
					var html = '<img sq_youtube_embeded="1" src="<?php echo $this->_img_url; ?>"';
					html += ' style="width: ' + param["f_width"] + 'px; height: ' + param["f_height"] + 'px; border: 2px dashed black;"';
					html += ' width="'        + param["f_width"] + '"';
					html += ' height="'       + param["f_height"] + '"';
					html += ' auto_start="'   + param["f_auto_start"] + '"';
					html += ' loop="'         + param["f_loop"] + '"';
					html += ' full_screen="'  + param["f_full_screen"] + '"';
					html += ' vid="'          + param["f_vid"] + '"';
					html += ' video_url="'    + param["f_video_url"] + '"';
					html += ' show_border="'  + param["f_show_border"] + '"';
					html += ' colour1="'      + param["f_colour1"] + '"';
					html += ' colour2="'      + param["f_colour2"] + '"';
					html += ' rel="'          + param["f_rel"] + '"';
					html += ' egm="'          + param["f_egm"] + '"';
					html += ' enable_js="'    + param["f_enable_js"] + '"';
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
		case "matrixembedyoutube":
			this._matrixEmbedYouTube();
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
		preg_match_all('|(<img[^\>]+sq_youtube_embeded="1"([^\>]+)?>)|i', $html, $matches);

		foreach ($matches[1] as $match) {
			$old_html = $match;

			//$match = strtolower($match);
			$match = substr($match, 4);
			$match = substr($match, 0, -1);
			$match = str_replace('src="'.$this->_img_url.'"', '', $match);
			$match = preg_replace('/style="(.*?("))/im ', '', $match);
			// IE7/8 strips off the quotes of attributes "height" and "weight" of "img" tag 
			// when inserting the html in the given range (Range.pasteHTML())
			$match = preg_replace('/ height=([0-9]+)/', ' height="$1"', $match);
			$match = preg_replace('/ width=([0-9]+)/', ' width="$1"', $match);

			$pieces = Array();
			preg_match_all('|\s+([^=]+)="([^"]+)"|', $match, $pieces);


			$movie_vars = Array();
			for (reset($pieces[1]); NULL !== ($k = key($pieces[1])); next($pieces[1])) {
				$var_name  = trim($pieces[1][$k]);
				$var_value = trim($pieces[2][$k]);
				$movie_vars[$var_name] = $var_value;
			}
			if (array_key_exists('video_url', $movie_vars) === TRUE) {
				$movie_vars['video_url'] = $movie_vars['video_url'].'&';
			}

			// Try and interpet the video id and create a valid url
			$movie_url_prefix = 'http://www.youtube.com/v/';
			$movie_url = '';
			$add_suffix_vars = TRUE;
			$current_video_id = '';
			if (isset($movie_vars['video_url']) && !empty($movie_vars['video_url'])) {
				if (preg_match('/youtu(\.?)be(.*)(v?)[\/|=](.+?)&/', $movie_vars['video_url'], $v_match)) {
					if (isset($v_match[4]) && !empty($v_match[4])) {
						// I found the video id
						$movie_url = $movie_url_prefix.$v_match[4];
						$current_video_id = $v_match[4];
					} else {
						// Pray, the url is correct
						$movie_url = $movie_vars['video_url'];
					}
				} else {
					$movie_url = $movie_vars['video_url'];
				}
			}
			// Do the video id last (this overrides all)
			if (isset($movie_vars['vid']) && !empty($movie_vars['vid'])) {
				$movie_url = $movie_url_prefix.$movie_vars['vid'];
			}

			// first try getting width and height from the style
			$width  = '';
			$height = '';
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

			$movie_url_vars = '?playerapiid=matrixytplayer';

			foreach($movie_vars as $var_name => $var_value) {
				switch ($var_name) {
					case 'colour1':
						if (!empty($var_value)) {
							$colour = str_replace('#', '', $var_value);
							$colour = strtolower($colour);
							$movie_url_vars .= '&color1=0x'.$colour;
						}
					break;
					case 'colour2':
						if (!empty($var_value)) {
							$colour = str_replace('#', '', $var_value);
							$colour = strtolower($colour);
							$movie_url_vars .= '&color2=0x'.$colour;
						}
					break;
					case 'rel':
						if ($var_value == '1') {
							$movie_url_vars .= '&rel='.$var_value;
						} else {
							$movie_url_vars .= '&rel=0';
						}
					break;
					case 'auto_start':
						if ($var_value == '1') {
							$movie_url_vars .= '&autoplay='.$var_value;
						} else {
							$movie_url_vars .= '&autoplay=0';
						}
					break;
					case 'loop':
						if ($var_value == '1') {
							$movie_url_vars .= '&loop='.$var_value.'&playlist='.$current_video_id;
						} else {
							$movie_url_vars .= '&loop=0';
						}
					break;
					case 'egm':
						if ($var_value == '1') {
							$movie_url_vars .= '&egm='.$var_value;
						} else {
							$movie_url_vars .= '&egm=0';
						}
					break;
					case 'show_border':
						if ($var_value == '1') {
							$movie_url_vars .= '&border='.$var_value;
						} else {
							$movie_url_vars .= '&border=0';
						}
					break;
					case 'full_screen':
						if ($var_value == '1') {
							$movie_url_vars .= '&fs='.$var_value;
						} else {
							$movie_url_vars .= '&fs=0';
						}
					break;
					case 'enable_js':
						if ($var_value == '1') {
							$movie_url_vars .= '&enablejsapi='.$var_value;
						} else {
							$movie_url_vars .= '&enablejsapi=0';
						}
					break;
				}//end switch
			}//end foreach

			$movie_url_vars = htmlentities($movie_url_vars, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

			if (strpos($movie_url, '?') !== FALSE) $movie_url_vars = ltrim($movie_url_vars, '?');

			ob_start();
			?>
			<sq_wysiwyg_embed_youtube <?php echo $match; ?>>
			<script src="http://swfobject.googlecode.com/svn/tags/rc3/swfobject/src/swfobject.js" type="text/javascript"></script>
			<?php
			//	$movie_vars['embed_loop']    = '0';
			?><object
			  <?php echo $width; ?> <?php echo $height; ?>>
				<param name="movie"            value="<?php echo $movie_url.$movie_url_vars; ?>">
				<param name="allowfullscreen"  value="<?php echo ($movie_vars['full_screen'])    ? 'true' : 'false';?>">
				<embed type="application/x-shockwave-flash"
					src="<?php echo $movie_url.$movie_url_vars; ?>"
					name="matrixytplayer"
					allowfullscreen="<?php echo ($movie_vars['full_screen'])            ? 'true' : 'false'; ?>"
					<?php echo $width; ?>
					<?php echo $height; ?>
				></embed>
			</object>
			</sq_wysiwyg_embed_youtube>
			<?php
			$new_html = ob_get_contents();
			ob_end_clean();

			$html = str_replace($old_html, $new_html, $html);

		}//end foreach $matches

		// make sure the htmlTidy plugin knows about our special tag
		if ($this->wysiwyg->plugin_loaded('html_tidy')) {
			$html_tidy =& $this->wysiwyg->get_plugin('html_tidy');
			$html_tidy->new_inline_tag('sq_wysiwyg_embed_youtube');
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
		$contents = preg_replace('|(<sq_wysiwyg_embed_youtube([^>]*)?>)(.*?)(</sq_wysiwyg_embed_youtube>)|si', '<IMG src="'.$this->_img_url.'" $2>', $contents);
		$contents = preg_replace('|\s+|', ' ', $contents);

	}//end set_contents()


}//end class

?>
