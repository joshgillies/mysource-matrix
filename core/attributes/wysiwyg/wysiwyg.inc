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
* $Id: wysiwyg.inc,v 1.44 2013/07/25 23:23:49 lwright Exp $
*
*/


require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';
require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';

/**
* WYSIWYG Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <wysiwyg style="div" width="100%" height="300" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_WYSIWYG extends Asset_Attribute_Text
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_WYSIWYG($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_Text($attributeid, $value);

		// Get the WYSIWYG style from the global preference as the default settings
		$wysiwyg_style = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_STYLE');
		$width_size_unit = isset($wysiwyg_style['units']['width']) ? $wysiwyg_style['units']['width'] : '';
		$height_size_unit = isset($wysiwyg_style['units']['height']) ? $wysiwyg_style['units']['height'] : '';

		// set default edit parameters
		$this->_edit_params['style']		= $wysiwyg_style['body_type'];
		$this->_edit_params['width']		= $wysiwyg_style['width'].$width_size_unit;
		$this->_edit_params['height']		= $wysiwyg_style['height'].$height_size_unit;
		$this->_edit_params['tidy_status']	= 'wait';
		$this->_edit_params['tidy_errors']	= '';

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates the params array from it
	*
	* @param object XML_Tree_Node	$node	the XML_Tree_Node from the edit interface XML file
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes()->style))  $this->_edit_params['style']  = (string)$node->attributes()->style;
		if (isset($node->attributes()->width))  $this->_edit_params['width']  = (string)$node->attributes()->width;
		if (isset($node->attributes()->height)) $this->_edit_params['height'] = (string)$node->attributes()->height;

		return true;

	}//end setEditParams()


	/**
	* Sets the various params for this WYSIWYG editor
	*
	* @param string	$prefix	prefix for form elements
	*
	* @return object WYSIWYG
	* @access private
	*/
	function &_createEditor($prefix)
	{
		// allowed plugins (shortened to $ap for easier use below)
		$ap = explode('|', $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_PLUGINS'));
		// ignore allowed plugins (shortened to $iap for easier use);
		$iap = in_array('all', $ap);

		$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');

		$wysiwyg->set_body_type($this->_edit_params['style']);
		$wysiwyg->set_init_onload(false);
		$wysiwyg->set_show_status_bar(false);
		$wysiwyg->set_width($this->_edit_params['width']);
		$wysiwyg->set_height($this->_edit_params['height']);

		// get the keyword replacements
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$keywords = Array();
		$keyword_vars = Array('keywords' => &$keywords);
		$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);

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

		// Snippet global keyword dropdown
		// However if in frontend do not include this plugin, as it will output global keywords
		$snippets = $GLOBALS['SQ_SYSTEM']->am->getSnippetKeywords();
		if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && ($iap || in_array('snippet_keyword_replace', $ap)) && !empty($snippets)) {
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

		$wysiwyg->set_contents($this->value);

		return $wysiwyg;

	}//end _createEditor()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			echo $this->value;
			return;
		}

		$this->_saveCssAssetIds();

		$wysiwyg = $this->_createEditor($prefix);

		//The following will only print in Safe Edit or EES
		//Hot fixes for old EES
		if (!(SQ_IN_BACKEND || SQ_IN_LIMBO)) {
			echo '<script type="text/javascript" src="'.sq_web_path('lib').'/js/edit.js"></script>';
			echo '<style>'
				.'.sq-wysiwyg-standalone-header table{width: 100%; margin: 0;}'
				.'.metadata .htmlarea-toolbar{margin: 0 !important;}'
				.'.metadata .rowWithWYSIWYG table{margin: 5px 0 0;}'
				.'.metadata div.htmlarea-separator div{border-left: 1px solid #B6B6B6 !important; border-right: 1px solid #FFF !important; margin: 0 !important;}'
				.'.htmlarea-toolbar td > div > div {margin: 0 1px 3px 0 !important;}'
				.'.metadata div.htmlarea-separator {margin: 0 3px !important;}'
			.'</style>';
		}
		?>
		<script type="text/javascript">
			var bodycopy_initialised = true;
		</script>
		<div id="<?php echo $prefix.'_switch_option_div'; ?>" class="sq-wysiwyg-standalone-header sq-grey-header-wrapper">
			<table style="width: 100%; margin: 0;">
				<tr class="sq-grey-header-row">
					<td align="left">
						<?php
						$o = $GLOBALS['SQ_SYSTEM']->backend->out;
						if(is_null($o) || !$o->isViperAvailable()) {
						?>
							<div style="width: 16px; height: 16px; background: url(<?php echo sq_web_path('lib')?>/web/images/icons/edit_mode.png);background: expression('none');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib')?>/web/images/icons/edit_mode.png', sizingMethod='scale'); font-size:10px; cursor:pointer;"
					onclick="switchEditingMode('<?php echo $prefix; ?>_contents_div','<?php echo $prefix; ?>_wysiwyg_div', editor_<?php echo $prefix; ?>);">
							</div>&nbsp;&nbsp;
						<?php 
						}
						echo '<strong>'.translate('wysiwyg_content_container').'</strong>';
						?>
					</td>
					<td align="right" class="sq-left-img-margin">
						<?php echo $this->paintBackendStatusIcons($wysiwyg); ?>
					</td>
				</tr>
			</table>
		</div>
		<div class="sq-wysiwyg-standalone-body with-bottom-margin">
		<?php
		$o = $GLOBALS['SQ_SYSTEM']->backend->out;
		if(!is_null($o) && $o->isViperAvailable()) {
		    $o->enableViperEditor();	    
		    // Use a hidden input to pass the Viper edited content to Matrix backend form
		    $o->addOnSubmit('document.getElementById("'.$prefix.'_contents_div_viper_input").value = document.getElementById("'.$prefix.'_contents_div_viper").innerHTML;');
		    ?>
		  <div id="<?php echo $prefix?>_contents_div_viper" class="with-viper"><?php echo $this->value;?></div>
		  <input  id="<?php echo $prefix?>_contents_div_viper_input" style="display:none;" name="<?php echo $prefix?>_contents_div_viper_input" value=""/>
		  <?php
		}
		else {
		?>
		    
			    <div style="display:none;" id="<?php echo $prefix?>_wysiwyg_div"><?php echo $wysiwyg->paint()?></div>
			    <div id="<?php echo $prefix?>_contents_div" class="sq-wysiwyg-preview-wrapper" onclick="editDiv=document.getElementById('<?php echo $prefix ?>_wysiwyg_div'); if (editDiv !== null && editDiv.style.display == 'none') {switchEditingMode('<?php echo $prefix ?>_contents_div','<?php echo $prefix ?>_wysiwyg_div', editor_<?php echo $prefix ?>);}"><?php echo $this->value?></div>
		    
		<?php
		}
		?>
		</div>
		<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string		$prefix			prefix for the form elements
	* @param int		$owner_id		id of the asset such as standard page and news item which own that html content
	* @param boolean	$cmp_value		whether to compare current value with new value	in the decision to process
	*
	* @access public
	*/
	function process($prefix, $owner_id=NULL, $cmp_value=TRUE)
	{
	    	if(isset($_REQUEST[$prefix.'_contents_div_viper_input'])) {
		    $html = $_REQUEST[$prefix.'_contents_div_viper_input'];
		}
		else {
		    $wysiwyg = $this->_createEditor($prefix);
		    $html = $wysiwyg->process();
		}
		
		$this->processed = FALSE;
		if ($html !== FALSE && (!$cmp_value || $html !== $this->value) && $this->setValue($html, $owner_id)) {
			$this->processed = TRUE;
		}
		$this->_edit_params['tidy_status'] = isset($wysiwyg->_loaded_plugins['html_tidy']->htmltidy_status) ? $wysiwyg->_loaded_plugins['html_tidy']->htmltidy_status : 'wait';
		$this->_edit_params['tidy_errors'] = isset($wysiwyg->_loaded_plugins['html_tidy']->htmltidy_errors) ? $wysiwyg->_loaded_plugins['html_tidy']->htmltidy_errors : '';

	}//end process()


	/**
	* Returns the content of this attribute as it should be shown in a keyword replacement
	*
	* @param string	$format	Format suffix
	*
	* @return void
	* @access public
	*/
	function getKeywordValue($format='')
	{
		if (!in_array($format, Array('', 'disableurls', 'disablekeywords', 'disableall'))) {
			return '';
		}

		$output = $this->value;

		if (!in_array($format, Array('disableurls', 'disableall'))) {
			$matches = Array();
			preg_match_all('|\\./\\?a=([0-9]+)([^0-9:])|', $output, $matches);
			$normal_assetids = empty($matches[1]) ? Array() : $matches[1];
			$matches = Array();
			preg_match_all('|\\./\\?a=([0-9]+:[^$]*)\\$|', $output, $matches, PREG_PATTERN_ORDER);
			$shadow_assetids = empty($matches[1]) ? Array() : $matches[1];
			$assetids = array_unique(array_merge($normal_assetids, $shadow_assetids));
			$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetUrl($assetids);
			$output = $this->value;
			foreach ($urls as $id => $url) {
				if (FALSE === strpos($id, ':')) {
					if (!empty($url)) {
						$output = str_replace('./?a='.$id, $url, $output);
					}
				} else {
					if (!empty($url)) {
						$output = str_replace('./?a='.$id.'$', $url, $output);
					} else {
						$output = str_replace('./?a='.$id.'$', './?a='.$id, $output);
					}
				}
			}
		}

		if (in_array($format, Array('disablekeywords', 'disableall'))) {
			// Simulate 'disable keywords' from bodycopy_container
			// by encoding %s
			$keywords = retrieve_keywords_replacements($output);
			foreach ($keywords as $keyword) {
				$output = str_replace("%$keyword%", "&#37;{$keyword}&#37;", $output);
			}
		}

		return $output;

	}//end getKeywordValue()


	/**
	* Checks for hyperlinks pointing to non-live Matrix assets
	* Sets the value of this attribute after validating
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function setValue(&$value, $owner_id = NULL)
	{
		if (!empty($owner_id)) {
			$live_link_only = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_LIVE_LINK_ONLY');
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($owner_id);
			if ($asset_info[$owner_id]['status'] == SQ_STATUS_LIVE && $live_link_only) {
				require_once SQ_FUDGE_PATH.'/general/www.inc';
				$outcome = check_non_live_link($value, $owner_id);
				if ($outcome !== FALSE) $value = $outcome;
			}
		}

		return parent::setValue($value);

	}//end setValue()


	/**
	* Prints extra backend icons at the right hand side of the menu
	*
	* @param Content_Type_WYSIWYG	$asset	the WYSIWYG Content Type object
	* @param string					$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintBackendStatusIcons($wysiwyg)
	{
		require_once SQ_DATA_PATH.'/private/conf/tools.inc';
		require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

		$o = $GLOBALS['SQ_SYSTEM']->backend->out;
		if(!is_null($o) && $o->isViperAvailable()) {
		    Bodycopy_Edit_Fns::printBodycopyIcon('', 'Viper Content Area', translate('viper_editor_enabled_icon_note'), 'viper_logo', '[Viper Content Area]');
		}
		else {
		    // prints the status of the last run of htmltidy tool as a tag icon
		    if (!SQ_TOOL_HTML_TIDY_ENABLED || !file_exists(SQ_TOOL_HTML_TIDY_PATH)) {
			    // tool is not enabled
			    Bodycopy_Edit_Fns::printBodycopyIcon('', 'HTMLTidy Status', translate('wysiwyg_htmltidy_status_disabled'), 'htmltidy_disabled', '[HTMLTidy Status]');
		    } else {

			    $status = $this->_edit_params['tidy_status'];
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

			    $res = (str_replace("\n", '<br />', htmlentities($this->_edit_params['tidy_errors'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));

			    $htmltidy_accessibility_filename = 'accessibility_'.(empty($res) ? 'pass' : 'fail');
		Bodycopy_Edit_Fns::printBodycopyIcon('', 'HTMLTidy Accessibility', !empty($res) ? $res : translate('wysiwyg_htmltidy_accessibility_no_warning'), $htmltidy_accessibility_filename, '[HTMLTidy Status]');

		    }
		}

	}//end paintBackendStatusIcons()


	/**
	* Save the ids of all css files attached to this url to be used by the wysiwyg editor later on
	*
	* @return void
	* @access private
	*/
	protected function _saveCssAssetIds()
    {
        if (!isset($GLOBALS['sq_bodycopy_cssids']) && isset($GLOBALS['sq_preview_url']) && !empty($GLOBALS['sq_preview_url'])) {
            $url = $GLOBALS['sq_preview_url'];
            $url = preg_replace('|^http[s]?://|', '', $url);

            $designid =  $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, 'design::%frontend%', TRUE);

            if ($designid) {
                $design = $GLOBALS['SQ_SYSTEM']->am->getAsset($designid);
                $stylesheets = $design->getStylesheets();

                // save the ids of all the css files into a global var so the wysiwyg style
                // drop down plugin can access this information without another querey
                // we can also limit the stylesheets that are available to it here if we want
                $GLOBALS['sq_bodycopy_cssids'] = array_keys($stylesheets);

                $GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
                unset($design);

            }
        }

	}//end _saveCssAssetIds()


}//end class

?>
