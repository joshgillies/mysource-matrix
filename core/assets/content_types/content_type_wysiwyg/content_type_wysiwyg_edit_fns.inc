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
* $Id: content_type_wysiwyg_edit_fns.inc,v 1.60 2013/07/25 23:23:48 lwright Exp $
*
*/


require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';
require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_WYSIWYG_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.60 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_WYSIWYG_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Sets the various params for this WYSIWYG editor, and returns a Fudge WYSIWYG object
	*
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	* @param array			$keywords   for keywords replacement plugin
	* @param array			$snippets   for snippets replacement plugin
	* 
	* @return WYSIWYG
	* @access private
	*/
	protected function _createEditor(Content_Type_WYSIWYG $asset, $prefix, $keywords=Array(), $snippet=Array())
	{
		// allowed plugins (shortened to $ap for easier use below)
		$ap = explode('|', $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_WYSIWYG_PLUGINS'));
		// ignore allowed plugins (shortened to $iap for easier use);
		$iap = in_array('all', $ap);

		$html = $asset->attr('html');

		if (empty($this->_tmp['wysiwg_object'])) {

			$wysiwyg = new WYSIWYG($prefix, sq_web_path('fudge').'/wysiwyg/');

			$wysiwyg_style = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_WYSIWYG_STYLE');
			$wysiwyg->set_body_type($wysiwyg_style['body_type']);
			$wysiwyg->set_init_onload(FALSE);
			$wysiwyg->set_show_status_bar(FALSE);
			$size_unit = '';
			if (is_numeric($wysiwyg_style['width']) && isset($wysiwyg_style['units']['width'])) {
				$size_unit = $wysiwyg_style['units']['width'];
			}
			$wysiwyg->set_width($wysiwyg_style['width'].$size_unit);
			$size_unit = '';
			if (is_numeric($wysiwyg_style['height']) && isset($wysiwyg_style['units']['height'])) {
				$size_unit = $wysiwyg_style['units']['height'];
			}
			$wysiwyg->set_height($wysiwyg_style['height'].$size_unit);


			$wysiwyg->open_group();

				if ($iap || in_array('format_text', $ap)) {
					$wysiwyg->add_plugin('format_text');
				}
				if ($iap || in_array('replace_text', $ap)) {
					$wysiwyg->add_plugin('replace_text');
				}
				if ($iap || in_array('search_and_replace', $ap)) {
					$wysiwyg->add_plugin('search_and_replace');
				}
				if ($iap || in_array('format_font', $ap)) {
					$wysiwyg->add_plugin('format_font');
				}
				if ($iap || in_array('general', $ap)) {
					$wysiwyg->add_plugin('general');
				}
				if ($iap || in_array('s_script', $ap)) {
					$wysiwyg->add_plugin('s_script');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
			if ($iap || in_array('matrix_apply_style', $ap)) {
					$wysiwyg->add_plugin('matrix_apply_style');
				}
			$wysiwyg->close_group();

			if ($iap || in_array('justify_text', $ap)) {
				$wysiwyg->add_plugin('justify_text');
			}
			if (($iap || in_array('keyword_replace', $ap)) && !empty($keywords)) {
				$wysiwyg->add_plugin('keyword_replace');
				$keyword_replace =& $wysiwyg->get_plugin('keyword_replace');
				$keyword_replace->keyword_tag = '%';
				$keyword_replace->keywords = $keywords;
			}

			// snippet global keyword dropdown
			if (($iap || in_array('snippet_keyword_replace', $ap)) && !empty($snippets)) {
				$wysiwyg->add_plugin('snippet_keyword_replace');
				$snippet_keyword_replace =& $wysiwyg->get_plugin('snippet_keyword_replace');
				$snippet_keyword_replace->keyword_tag = '%';
				$snippet_keyword_replace->keywords = $snippets;
			}

			if ($iap || in_array('history', $ap)) {
				$wysiwyg->add_plugin('history');
			}
			if ($iap || in_array('indenting', $ap)) {
				$wysiwyg->add_plugin('indenting');
			}
			if ($iap || in_array('listing', $ap)) {
				$wysiwyg->add_plugin('listing');
			}
			if ($iap || in_array('table_editing', $ap)) {
				$wysiwyg->add_plugin('table_editing');
			}
			if ($iap || in_array('select_color', $ap)) {
				$wysiwyg->add_plugin('select_color');
			}

			$wysiwyg->open_group();
				if ($iap || in_array('matrix_insert_link', $ap) || in_array('insert_link', $ap)) {
					$wysiwyg->add_plugin('matrix_insert_link');
				}
				if ($iap || in_array('matrix_insert_quick_link', $ap)) {
					$wysiwyg->add_plugin('matrix_insert_quick_link');
				}
				if ($iap || in_array('matrix_insert_anchor', $ap) || in_array('insert_anchor', $ap)) {
					$wysiwyg->add_plugin('matrix_insert_anchor');
				}
				if ($iap || in_array('matrix_insert_image', $ap) || in_array('insert_image', $ap)) {
					$wysiwyg->add_plugin('matrix_insert_image');
				}
				if ($iap || in_array('matrix_embed_movie', $ap)) {
					$wysiwyg->add_plugin('matrix_embed_movie');
				}
				if ($iap || in_array('matrix_embed_youtube', $ap)) {
					$wysiwyg->add_plugin('matrix_embed_youtube');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('matrix_accessibility', $ap)) {
					$wysiwyg->add_plugin('matrix_accessibility');
				}
			$wysiwyg->close_group();

			// html_tidy needs to be last so other plugins can set options
			if ($iap || in_array('html_tidy', $ap)) {
				$wysiwyg->add_plugin('html_tidy');
			}

			if ($iap || in_array('matrix_expand_keywords', $ap)) {
				$wysiwyg->add_plugin('matrix_expand_keywords');
			}

			$wysiwyg->open_group();
				if ($iap || in_array('special_chars', $ap)) {
					$wysiwyg->add_plugin('special_chars');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('choose_language', $ap)) {
					$wysiwyg->add_plugin('choose_language');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('definition_list', $ap)) {
					$wysiwyg->add_plugin('definition_list');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('misc_accessibility', $ap)) {
					$wysiwyg->add_plugin('misc_accessibility');
				}
				if ($iap || in_array('quotation', $ap)) {
					$wysiwyg->add_plugin('quotation');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('visual_aid', $ap)) {
					$wysiwyg->add_plugin('visual_aid');
				}
			$wysiwyg->close_group();

			$wysiwyg->open_group();
				if ($iap || in_array('spell_checker', $ap)) {
					$wysiwyg->add_plugin('spell_checker');
				}
				if ($iap || in_array('text_indicator', $ap)) {
					$wysiwyg->add_plugin('text_indicator');
				}
				if ($iap || in_array('view_source', $ap)) {
					$wysiwyg->add_plugin('view_source');
				}
			$wysiwyg->close_group();
			
			$this->_tmp['wysiwg_object'] =& $wysiwyg;

		}//end if WYSIWYG object doesn't already exist

		// contents must be set AFTER all plugins are loaded
		$this->_tmp['wysiwg_object']->set_contents($html);
		
		// set dir attribute to include in wysiwyg iframe
		if(!empty($this->_tmp['dir_attr']))
			$this->_tmp['wysiwg_object']->set_dir_attr($this->_tmp['dir_attr']);
		
		return $this->_tmp['wysiwg_object'];

	}//end _createEditor()


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	* @param Backend_Outputter	$o		the Backend Outputter instance
	*
	* @return boolean
	* @access public
	*/
	public function paintBackend(Content_Type_WYSIWYG $asset, $prefix, $o=NULL)
	{
		if ($asset->writeAccess('attributes')) {
		    	// get the keyword replacements
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$keywords = Array();
			$keyword_vars = Array('keywords' => &$keywords);
			$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);
			$keywords_encoded = htmlentities(json_encode_array($keywords));

			// get snippet replacements
			$snippets = $GLOBALS['SQ_SYSTEM']->am->getSnippetKeywords();
			$snippets_encoded = htmlentities(json_encode_array($snippets));
			    
			if(!is_null($o) && $o->isViperAvailable()) {
			    
			    $o->enableViperEditor();

				// check to see what plugins are enabled/disabled for the Viper Editor
				$viper_plugins = explode('|', $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_VIPER_PLUGINS'));
				
				if (in_array('MatrixLinkPlugin', $viper_plugins)) $viper_plugins[] = 'ViperLinkPlugin';

				// if all the plugins are enabled then we need not worry about anything
				if (in_array('all', $viper_plugins) !== TRUE) {
					if (count($viper_plugins) > 0) {
						// If we have at least one plugin selected, add the four "core
						// plugins" to the list.
						$core_plugins = Array(
							'ViperKeyboardEditorPlugin',
							'ViperToolbarPlugin',
							'ViperInlineToolbarPlugin',
							'ViperCursorAssistPlugin',
						);
						$viper_plugins = array_merge($core_plugins, $viper_plugins);
					}
					$string_for_js = "['".implode("', '", $viper_plugins)."']";
					?>
						<script type="text/javascript"><!--
							//<![CDATA[
								// this function need only be executed after the whole DOM is loaded
								// which would enable our Viper editor to load and be ready for us
								var PluginsEnabled = <?php echo $string_for_js; ?>;
							//]]> -->
						</script>
					<?php
				}

			    // Clean the HTML before passing to the Matrix backend form
			    $o->addOnSubmit('Matrix_Viper.viper.cleanDOM(document.getElementById("'.$prefix.'_contents_div_viper"));');
			    // Use a hidden input to pass the Viper edited content to Matrix backend form
			    $o->addOnSubmit('document.getElementById("'.$prefix.'_contents_div_viper_input").value = document.getElementById("'.$prefix.'_contents_div_viper").innerHTML;');
			    ?>
			    <div id="<?php echo $prefix?>_contents_div_viper" class="with-viper" data-keywords="<?php echo $keywords_encoded; ?>" data-snippets="<?php echo $snippets_encoded; ?>"><?php echo $asset->attr('html');?></div>
			    <textarea  id="<?php echo $prefix?>_contents_div_viper_input" style="display:none;" name="<?php echo $prefix?>_contents_div_viper_input" ></textarea>
			   <?php
			}
			else {
			    $wysiwyg = $this->_createEditor($asset, $prefix, $keywords, $snippets);
			    ?>
			  <div style="display:none;" id="<?php echo $prefix?>_wysiwyg_div"><?php echo $wysiwyg->paint(); ?></div>
			  <div id="<?php echo $prefix?>_contents_div"   data-keywords="<?php echo $keywords_encoded; ?>" data-snippets="<?php echo $snippets_encoded; ?>" onclick="editDiv=document.getElementById('<?php echo $prefix ?>_wysiwyg_div'); if (editDiv !== null && editDiv.style.display == 'none') {switchEditingMode('<?php echo $prefix ?>_contents_div','<?php echo $prefix ?>_wysiwyg_div', editor_<?php echo $prefix ?>);}" class="htmlarea-div"><?php echo $wysiwyg->contents ?></div>
			  <?php
			}
		}
		return TRUE;

	}//end paintBackend()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	* @param Backend_Outputter	$o		the Backend Outputter instance
	*
	* @return void
	* @access public
	*/
	public function paintBackendIcons(Content_Type_WYSIWYG $asset, $prefix, $o=NULL)
	{
		if(!is_null($o) && !$o->isViperAvailable())
		    Bodycopy_Edit_Fns::printBodycopyIcon("if(typeof editor_$prefix !== 'undefined') {switchEditingMode('".$prefix."_contents_div','".$prefix."_wysiwyg_div', editor_".$prefix.');}', 'Edit Contents', 'Switch between preview and edit mode', 'edit_mode', '[Edit Contents]');

	}//end paintBackendIcons()


	/**
	* Prints extra backend icons at the right hand side of the menu
	*
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	* @param Backend_Outputter	$o		the Backend Outputter instance
	*
	* @return void
	* @access public
	*/
	public function paintBackendStatusIcons(Content_Type_WYSIWYG $asset, $prefix, $o=NULL)
	{
		if(!is_null($o) && $o->isViperAvailable()) {
		     //Bodycopy_Edit_Fns::printBodycopyIcon('', 'Edit+ Content Area', translate('edit+_editor_enabled_icon_note'), 'viper_logo', '[Edit+ Content Area]');
		}
		else {
		    require_once SQ_DATA_PATH.'/private/conf/tools.inc';

		    // prints the status of the last run of htmltidy tool as a tag icon
		    if (!SQ_TOOL_HTML_TIDY_ENABLED || !file_exists(SQ_TOOL_HTML_TIDY_PATH)) {
			    // tool is not enabled
			    Bodycopy_Edit_Fns::printBodycopyIcon('', 'HTMLTidy Status', translate('wysiwyg_htmltidy_status_disabled'), 'htmltidy_disabled', '[HTMLTidy Status]');
		    } else {

			    $status = $asset->attr('htmltidy_status');
			    $status_msg = translate('wysiwyg_htmltidy_status_wait');
			    $image_filename = 'htmltidy_'.$status;
			    switch ($status) {
				    case 'pass':
					    $status_msg = translate('wysiwyg_htmltidy_status_pass');
				    break;
				    case 'fail':
					    $status_msg = translate('wysiwyg_htmltidy_status_fail');
				    break;
				    case 'wait':
					    $status_msg = translate('wysiwyg_htmltidy_status_wait');
				    break;
			    }//end switch
			    Bodycopy_Edit_Fns::printBodycopyIcon('', 'HTMLTidy Status', $status_msg, $image_filename, '[HTMLTidy Status]');

			    // Htmltidy errors seperated by newline will break the javascript underneath in printBodycopyIcon() call.
			    $res = (str_replace("\n", '<br />', htmlentities($asset->attr('htmltidy_errors'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));

			    $htmltidy_accessibility_filename = 'accessibility_'.(empty($res) ? 'pass' : 'fail');
		Bodycopy_Edit_Fns::printBodycopyIcon('', 'HTMLTidy Accessibility', !empty($res) ? $res : translate('wysiwyg_htmltidy_accessibility_no_warning'), $htmltidy_accessibility_filename, '[HTMLTidy Status]');

		    }
		}

	}//end paintBackendStatusIcons()


	/**
	* Paints raw HTML
	*
	* @param Content_Type_WYSIWYG	$asset		the WYSIWYG Content Type object
	* @param boolean				$editing	are we printing an editing interface
	* @param boolean				$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	public function paint(Content_Type_WYSIWYG $asset, $editing=FALSE, $generating=FALSE)
	{
		$html = $asset->attr('html');

		// remove any sq_wysiwyg_* tags used as placeholders for plugins
		// such as Embed Movie/Youtube
		if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			$html = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $html);
		}

		$html = preg_replace('|(<htmlarea_special[^>]*?>)(.*?)(</htmlarea_special>)|si', '$2', $html);
		if ($generating) {
			// escape dangerous content
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			escape_php($html);
			echo $html;
			return TRUE;
		}

		if ($editing) {
			// We are in the edit interface, so print the interface and content
			$this->_paintInterface($html, $asset);
		} else {
			// Just print content
			echo $html;
		}//end if

	}//end paint()


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array					$link	information used to create the initial link
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	* @param Backend_Outputter	$o		the Backend Outputter instance
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Array $link, Content_Type_WYSIWYG $asset, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		
		if(isset($_REQUEST[$prefix.'_contents_div_viper_input'])) {
		    $html = $_REQUEST[$prefix.'_contents_div_viper_input'];
		}
		else {
		    $wysiwyg = $this->_createEditor($asset, $prefix);
		    $html = $wysiwyg->process();
		}
		if ($html === FALSE) return FALSE;

		if (!isset($_POST['ees_content_no_filter']) && isset($wysiwyg)) {
			// grab and store the last run status of htmltidy plugin
			// to be used when we print the corresponding status icon
			$html_tidy =& $wysiwyg->get_plugin('html_tidy');
			if (!is_null($html_tidy)) {
				$asset->setAttrValue('htmltidy_status', $html_tidy->htmltidy_status);
				$asset->setAttrValue('htmltidy_errors', $html_tidy->htmltidy_errors);
			}//end if
		}

		if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && !headers_sent()) {
			// Without this header disabled, Chrome will not output the iframe's source in the editor
			header('X-XSS-Protection: 0');
		}

		return $asset->setAttrValue('html', $html);

	}//end processBackend()
	
	
	/**
	* Sets the tmp var for dir attribute
	*
	* @param string					$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function setDirAttr($dir_attribute)
	{
		$this->_tmp['dir_attr'] = $dir_attribute;
		
	}//end setDirAttr()
	

}//end class

?>
