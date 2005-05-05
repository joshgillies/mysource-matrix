<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_template_builder.inc,v 1.13 2005/05/05 06:20:47 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Template_Builder
*
* Purpose
*	Creates bodycopy assets by collecting values from the user and subsituting them into a template
*	that was set up in the backend
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Template_Builder extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Template_Builder($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Folders set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;
		return $this->createBodycopy('Template Bodycopy', 'template', '', null, true);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_2 => Array('bodycopy' => Array('card' => 'M', 'exclusive' => false)));

	}//end _getAllowedLinks()


	/**
	* Creates a bodycopy linked underneath this asset with specified name, value and text
	*
	* @param string	$name		the name of the bodycopy asset
	* @param string $link_value	the value attached to the link
	* @param string $bc_text	the bodycopy text to be placed inside the first bodycopy DIV
	* @param string $underneath the asset which created bodycopies will be created
	*							underneath (defaults to underneath the Template Builder	itself)
	*
	* @return boolean	true indicates success
	* @access public
	*/
	function createBodycopy($name, $link_value, $bc_text='', $underneath=null, $is_dependant=false)
	{
		if ($underneath == null) $underneath =& $this;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bc_asset = new Bodycopy();

		$copy_link = Array(
						'asset'			=> &$underneath, 
						'link_type'		=> SQ_LINK_TYPE_2, 
						'value'			=> $link_value, 
						'sort_order'	=> 1,
						'is_dependant'	=> $is_dependant
					 );
		$bc_asset->setAttrValue('name', $name);
		$attributes = Array('content' => $bc_text);
		if (!$bc_asset->create($copy_link, $attributes)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $bc_asset;

	}//end createBodycopy()


	/**
	* Get the locations that the bodycopy should be linked under
	*
	* @access protected
	* @return array	list of assetids
	*/
	function getCreateLocations()
	{
		$res = $this->attr('create_locations') + $this->getAdditionalCreateLocations();
		return $res;

	}//end getCreateLocations()


	/**
	* Get a list of additional create locations from the REQUEST var
	*
	* Also ensures the passed additional locations are valid by checking the allowed
	* additional create locations that have been set in the asset
	*
	* @return array(parentid => link_type)
	* @access protected
	*/
	function getAdditionalCreateLocations()
	{
		$res = Array();
		if (isset($_REQUEST['ADDITIONAL_LOCATIONS']) && !empty($_REQUEST['ADDITIONAL_LOCATIONS'])) {
			// set the assetids and link types from the query
			foreach (explode(';', $_REQUEST['ADDITIONAL_LOCATIONS']) as $loc_code) {
				if (false === strpos($loc_code, '-')) {
					$link_type = SQ_LINK_TYPE_1;
					$loc_id = $loc_code;
				} else {
					list($loc_id, $link_type) = explode('-', $loc_code);
				}
				if (!isset($res[$loc_id]) || ($res[$loc_id] > $link_type)) {
					$res[$loc_id] = $link_type;
				}
			}
			// check that the types and tree locations are valid
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($res));
			/* Uncomment to enable type checking
			$valid_types = $this->attr('additional_location_types');
			*/
			$valid_parents = $this->attr('additional_location_parents');
			foreach ($infos as $assetid => $info) {
				/*
				uncomment this bit when you want to enable parent type checking
				if (!in_array($info['type_code'], $valid_types)) {
					$matches = array_intersect($GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($info['type_code']), $valid_types);
					if (empty($matches)) {
						unset($res[$assetid]);
					} else {
						// save us looking it up again
						$valid_types[] = $info['type_code'];
					}
				}
				*/
				if (!in_array($assetid, $valid_parents)) {
					$matches = array_intersect(array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($assetid)), $valid_parents);
					if (empty($matches)) {
						unset($res[$assetid]);
					}
				}
			}
		}
		return $res;

	}//end getAdditionalCreateLocations()


	/**
	* Returns information about the question fields currently in the template bodycopy
	*
	* Format of returned array is:
	* <PRE>
	* Array('raw'    => string (actual replacement code, including hashes etc)
	*		'name'   => string (name of field),
	*		'type'	 => string (currently type of bracketed expression - '[' or '{'),
	*		'values' => Array  (exploded list of possible values)
	* </PRE>
	*
	* @return Array
	* @access public
	*/
	function &getQuestionFields()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$link =& reset($links);

		$bc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);

		$bc_text = $bc_asset->getRawBodycopyContent();

		preg_match_all('/#(.*)(((\[)(.*)\]|(\{)(.*)\}|(\()(.*)\)))?#/U', $bc_text, $matches, PREG_SET_ORDER);

		$fields = Array();

		foreach ($matches as $bc_fields) {
			switch (count($bc_fields)) {
				// eg. #Text Field(50x3)#
				// everything specified - freeform text, size specified
				case 10:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[8],
										'values' => explode('x', $bc_fields[9]),
										'raw'    => $bc_fields[0],
										'is_select' => false,
									);
				break;
				// eg. #Multiple Selection{Option One|Option Two|Option Three}#
				// everything specified - multiple selection field
				case 8:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[6],
										'values' => explode('|', $bc_fields[7]),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;

				// eg. #Single Selection[Option One|Option Two|Option Three]#
				// everything specified - single selection field
				case 6:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[4],
										'values' => explode('|', $bc_fields[5]),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;

				// eg. #Multiple Selection{}# or #Single_Selection[]#
				// bracket specified, no contents!!
				case 5:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[4],
										'values' => Array(),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;

				// eg. #Text Field#
				// no bracket specified at all = freeform text with default 40x1 size
				default:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => '',
										'values' => Array(),
										'raw'    => $bc_fields[0],
										'is_select' => false,
									);
				break;

			}//end switch

		}

		return $fields;

	}//end getQuestionFields()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	* @return void
	*/
	function printBody()
	{
		if (!isset($_POST['SQ_TEMPLATE_COMMIT'])) {
			echo $this->paintDefaultForm();
		} else {
			// process
			$res = $this->getTemplateResult();
			echo $res;
			echo $this->paintPrintButton($res);
			$bc_asset = null;
			$create_locations = $this->getCreateLocations();
			if (empty($create_locations)) {
				trigger_localised_error('CMS0012', E_USER_WARNING);
				return;
			}
			foreach ($create_locations as $assetid => $link_type) {
				$under_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($bc_asset)) {
					$bc_asset =& $this->createBodycopy(date('d M Y H:i:s'), '', $res, $under_asset);
					if (!$bc_asset) {
						trigger_localised_error('CMS0013', E_USER_WARNING, $under_asset->attr('name'), $under_asset->id);
						return;
					}
				} else {
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($under_asset, $bc_asset, SQ_LINK_TYPE_2)) {
						trigger_localised_error('CMS0013', E_USER_WARNING, $under_asset->attr('name'), $under_asset->id);
						return;
					}
				}
			}
			if (!$bc_asset->processStatusChange($this->attr('create_status'))) {
				trigger_localised_error('CMS0014', E_USER_WARNING);
			}
		}

	}//end printBody()


	/**
	* Paint a button to print a printer-friendly version of the result
	*
	* @param string	$content	The content for printing
	*
	* @access private
	* @return void
	*/
	function paintPrintButton($content)
	{
		?>
		<script type="text/javascript">
		<!--
			function print_result() {
				var alert_win = window.open('', 'Print_Preview', 'resizable=yes,scrollbars=yes,width=800,height=500');
				alert_win.document.open();
				alert_win.document.writeln('<html><body style="margin: 25mm; width: 160mm; height: 247mm;">');
				alert_win.document.writeln('<?php echo str_replace("\r", '', str_replace("\n", '\\n', addslashes($content))); ?>');
				alert_win.document.writeln("</body></html>");
				alert_win.document.close();
				alert_win.print();
			}
		// -->
		</script>
		<input type="button" value="Print" onclick="print_result()" />
		<?php

	}//end paintPrintButton()


	/**
	* Returns the output required to paint the default form
	*
	* @access public
	* @return string
	*/
	function paintDefaultForm()
	{
		$q_fields =& $this->getQuestionFields();
		$prefix = $this->type().'_'.$this->id;

		$output = '<form action="'.replace_query_string_vars(Array()).'" method="post"><table>';
		$parameter_map = &$this->getAttribute('parameter_map');
		if (($additional_locs = $parameter_map->getParameterValue('additional_locations')) != null) {
			$output .= '<input type="hidden" name="ADDITIONAL_LOCATIONS" value="'.$additional_locs.'" />';
		}

		foreach ($q_fields as $q_field) {

			$field_name = $prefix.'_'.strtolower(str_replace(' ','_',$q_field['name']));

			$output .= '<tr><td style="vertical-align: top">';
			$output .= $q_field['name'];
			$output .='</td><td style="vertical-align: top">';

			switch ($q_field['type']) {
				// single select - drop down OR radio buttons if too long
				case '[':
					$max_length = 0;
					foreach ($q_field['values'] as $q_value) {
						$max_length = max($max_length, strlen($q_value));
					}

					ob_start();

					if ($max_length > 39) {
						foreach ($q_field['values'] as $key => $q_value) {
							radio_button($field_name, $key, false);
							echo $q_value.'<br/>';
						}
					} else {
						combo_box($field_name, $q_field['values'], false, Array());
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;

				// multiple select, check boxes always
				case '{':
					ob_start();
					foreach ($q_field['values'] as $key => $q_value) {
						check_box($field_name.'[]', $key, false);
						echo $q_value.'<br/>';
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;

				// freeform text - size specified
				case '(':
					ob_start();
					if ($q_field['values'][1] == 1) {
						text_box($field_name, '', $q_field['values'][0]);
					} else {
						text_area($field_name, '', $q_field['values'][0], $q_field['values'][1]);
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;

				// freeform text - default size of 40 columns x single row
				case '':
					ob_start();
					text_box($field_name, '', 40);
					$output .= ob_get_contents();
					ob_end_clean();
				break;

				default:
					$output .= '* Unknown *';
				break;

			}//end switch


			$output .= '</td></tr>';
		}

		$output .= '<tr><td>';
		$output .= '&nbsp;</td><td>';
		$output .= '<input type="hidden" name="SQ_TEMPLATE_COMMIT" value="true">';
		$output .= '<input type="submit" value="Submit">';
		$output .= '<input type="reset" value="Clear">';
		$output .= '</td></tr>';
		$output .= '</table></form>';

		return $output;

	}//end paintDefaultForm()


	/**
	* Throws keyword replacements into the template bodycopy and returns the contents
	*
	* @return string
	* @access public
	*/
	function getTemplateResult()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$link =& reset($links);

		$bc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);

		$bc_text = $bc_asset->getRawBodycopyContent();

		$q_fields = $this->getQuestionFields();
		$prefix = $this->type().'_'.$this->id;

		foreach ($q_fields as $q_field) {
			$rep = $q_field['raw'];
			$field_name = $prefix.'_'.strtolower(str_replace(' ','_',$q_field['name']));

			if ($q_field['is_select']) {
				$field_index = array_get_index($_POST, $field_name, null);

				if (!is_null($field_index)) {

					if (is_array($field_index)) {
						if (count($field_index) == 0) {
						} else if (count($field_index) == 1) {
							// multiple selection field with only one selection
							$field_value = $q_field['values'][$field_index[0]];
						} else {
							// multiple selection field with more than one selection
							$field_values = Array();
							foreach ($field_index as $key => $value) {
								$field_values[] = $q_field['values'][$value];
							}

							$field_value = implode(', ', array_slice($field_values,0,-1));
							// $value still contains the last known value within the foreach
							$field_value .= ' and '.$q_field['values'][$value];
						}
					} else {
						// single selection field
						$field_value = $q_field['values'][array_get_index($_POST, $field_name, '')];
					}

				} else {
					$field_value = '';
				}

			} else {
				// freeform field
				$field_value = nl2br(htmlspecialchars(array_get_index($_POST, $field_name, '')));
			}

			$bc_text = str_replace($rep, $field_value, $bc_text);
		}

		return $bc_text;

	}//end getTemplateResult()


	/**
	* Return the keywords for this asset type with their replacements for this asset instance
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return array
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = parent::getAssetKeywords($descriptions);
		$keywords['template_bodycopy_id']['value'] = '';
		if ($this->id) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'template');
			if (!empty($link)) {
				$keywords['template_bodycopy_id']['value'] = $link['minorid'];
			}
		}
		return $keywords;

	}//end getAssetKeywords()


}//end class
?>