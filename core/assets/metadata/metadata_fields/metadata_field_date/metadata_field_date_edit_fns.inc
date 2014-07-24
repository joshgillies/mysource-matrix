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
* $Id: metadata_field_date_edit_fns.inc,v 1.51 2013/07/25 23:23:48 lwright Exp $
*
*/

require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Date_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.51 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end Metadata_Field_Date_Edit_Fns()


	/**
	* Paint the interface for configuring display format
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDateFormat(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		$attr = $asset->getAttribute('format');
		$attr->paint($prefix.'_format', !$asset->writeAccess('attributes'));
		ob_start();
			?><a href="<?php echo sq_web_path('data'); ?>/asset_types/metadata_field_date/popups/keywords.html" onClick="var win = window.open(this.href, 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=640,height=520'); return false;"><?php echo translate('click_here'); ?></a><?php
			$replacements_link = ob_get_contents();
		ob_end_clean();

		$o->note(translate('metadata_date_object_keys', $replacements_link));

	}//end paintDateFormat()


	/**
	* Process the interface for configuring display format
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDateFormat(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		$attr = $asset->getAttribute('format');
		if ($asset->writeAccess('attributes')) {
			$attr->process($prefix.'_format');
			if ($attr->processed) {
				return $asset->setAttrValue('format', $attr->value);
			}
		}
		return FALSE;

	}//end processDateFormat()


	/**
	* Print the interface for editing the time offset
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintOffset(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		$offset = $asset->attr('offset');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_offset', $offset, 30);
		} else {
			 echo $offset;
		}

	}//end paintOffset()


	/**
	* Processes the interface for editing the time offset
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processOffset(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_offset'])) return FALSE;
		return $asset->setAttrValue('offset', $_POST[$prefix.'_offset']);

	}//end processOffset()



	/**
	* Print the interface for editing the default value
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDefaultValue(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$default = $asset->attr('default');
		} else {
			$default = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$default = $default[$asset->id];
		}
		$wa = $asset->writeAccess('attributes');
		$this->paintValueInterface($asset, $o, $default, $wa, FALSE);
		return $wa;

	}//end paintDefaultValue()


	/**
	* Process the interface for editing the default value
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDefaultValue(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$is_contextable = (boolean)$asset->attr('is_contextable');
			$assetid = $asset->id;

			if ($is_contextable === TRUE) {
				$default = $asset->attr('default');
			} else {
				$default = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($assetid), 0);
				$default = $default[$assetid];
			}

			$field_cascade_value=FALSE;			
			$processed = FALSE;

			$prefix = $asset->getPrefix();
			$value = array_get_index($_POST, $prefix.'_repkeys', '');
			
			if (!empty($value)) {
				$default = '%'.trim($value).'%';
				$processed = TRUE;
			} else if (!empty($_POST[$prefix.'_duration']) && $_POST[$prefix.'_duration'] != '0'  && $_POST[$prefix.'_period'] != '-') {
				$value = trim(date('Y-m-d H:i:s', strtotime($_POST[$prefix.'_duration'].' '.$_POST[$prefix.'_period'])));
				$default = trim($value);
				$processed = TRUE;
			} else {
				$field = $asset->_getDateTimeField($prefix);
				if ($field->processField()) {
					$value = $field->value;
					$default = trim($value);
					$processed = TRUE;
				}
			}

			if (isset($_POST[$prefix.'_cascade_value'])) {
				$field_cascade_value = TRUE;				
			}			

			if ($is_contextable === FALSE) {
				$GLOBALS['SQ_SYSTEM']->changeContext(0);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}

			$asset->setAttrValue('default', $default);
			$asset->saveAttributes();

			if ($is_contextable === FALSE) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}
			
			return $processed;
		}
		return FALSE;

	}//end processDefaultValue()


	/**
	* Print the interface for editing how the editing interface parameters for the date time field that is used
	* to enter the metadata value for an individual asset
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintSetupParams(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		if ($asset->writeAccess('attributes')) {
			// custom definition for edit interface
			$arr = Array('s', 'n', 'c');
			$field->printEditOptions($arr);
		} else {
			echo $field->format();
		}

		$o->note(translate('metadata_setup_table_explanation'));

	}//end paintSetupParams()


	/**
	* Processes the interface for editing the Setup Params
	*
	* @param Metadata_Field_Date	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processSetupParams(Metadata_Field_Date $asset, Backend_Outputter $o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		if ($asset->writeAccess('attributes') && $field->processEditOptions()) {
			return $asset->setAttrValue('setup_params', $field->parameters);
		}

		return FALSE;

	}//end processSetupParams()


	/**
	* Paint the interface for filling in metadata values
	*
	* Returns true if a submitable interface has been printed
	*
	* @param Metadata_Field_Date	$asset			the asset whose interface we are painting
	* @param Backend_Outputter		$o				the outputter class
	* @param string					$value_str		the current value for this field in the owning asset (NOT $asset)
	*												NULL means that there is no current value set
	* @param boolean				$write_access	whether to allow editing of this field
	*												(typically depends on write access to owner and editability of field)
	* @param boolean				$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Date $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{

		$prefix = $asset->getPrefix();
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		$is_default = FALSE;
		$default_val = $asset->attr('default');

		if (is_null($value_str)) {
			$value_str = $default_val;
			$is_default = TRUE;
		}

		$value = '';
		$value_components = $asset->attr('value_components');
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		$select_options = Array(
							''					=> '-- '.strtolower(translate('keywords')).' --',
							'asset_created'		=> translate('creation_date'),
							'asset_updated'		=> translate('last_updated_date'),
							'asset_published'	=> translate('last_published_date'),
						  );

		// checking if the value is a keyword replacement
		$uses_keyword = (substr($value, 0, 1) == '%');
		if (empty($value)) $value = NULL;
		$field = $asset->_getDateTimeField($prefix, $value);

		if ($asset->attr('show_keywords')) {
			$field->parameters['extras'] = Array(
											'd'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											'm'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											'y'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											'h'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											'm'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											's'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
											'i'	=> 'onchange="document.getElementById(\''.$prefix.'_repkeys\').value = \'\'"',
										   );
		}
		if ($write_access) {
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/metadata_field_date/js/metadata_field_date.js"></script>
			<?php
			echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
						if ((substr($default_val, 0, 1) == '%') || empty($value)) {
							$default_date_exp = 'null';
							$default_kw_exp = $asset->attr('show_keywords') ? "'".trim($default_val, '%')."'" : 'null';
						} else {
							list($dy, $dm, $dd, $dh, $di, $ds) = sscanf($default_val, '%d-%d-%d %d:%d:%d');
							// bug fix 4653 Default metadata checkbox generates JS error when defaults are empty
							// make sure the the values are not NULL , if so we will have to take care of the default checkbox
							if($dy == NULL && $dm == NULL && $dd == NULL && $dh == NULL && $di == NULL && $ds == NULL) {
								$default_date_exp = 'null';
							} else {
								if ($dh == NULL) {
									$dh = '00';
								}
								if ($di == NULL) {
									$di = '00';
								}
								if ($ds == NULL) {
									$ds = '00';
								}
								$default_date_exp = "{y:$dy, m:$dm, d:$dd, h:$dh, i:$di, s:$ds}";
							}
							$default_kw_exp = 'null';
						}

						check_box($prefix.'_default', '1', $is_default, 'handleMetadataDatetimeDefaultClick(this, \''.$prefix.'\', '.$default_date_exp.', '.$default_kw_exp.')');
						label(translate('use_default'), $prefix.'_default');
						?>
						<?php
					echo '</span>';
				}

				if ($write_access && $print_cascade_values_option) {
					echo '<span class="sq-metadata-cascade-wrapper">';
						check_box($prefix.'_cascade_value', '1', FALSE);
						label(translate('cascade_value'), $prefix.'_cascade_value');
					echo '</span>';
				}
			echo '</span>';

			echo '<div class="sq-metadata-date-wrapper">';
				echo '<span class="sq-inline-fields-wrapper bottom-margin">';
					$field->printField();
				echo '</span>';
				echo '<br /> <span class="sq-inline-fields-wrapper bottom-margin"><b style="display: inline-block; width: 47px">-'.strtoupper(translate('or_in')).'- </b>';
				preg_match('/([0-9]+)\s([i|h|d|w|m|y])/', $value, $matches);
				text_box($prefix.'_duration', '0', 2, '', TRUE);
				echo ' ';
				$periods = Array(
							'-'	=> '--Select One --',
							'minutes'	=> translate('minutes'),
							'hours'		=> translate('hours'),
							'days'		=> translate('days'),
							'weeks'		=> translate('weeks'),
							'months'	=> translate('months'),
							'years'		=> translate('years'),
						   );

				combo_box($prefix.'_period', $periods, FALSE, '-', NULL);
				echo '<b> from now</b></span>';

				if ($asset->attr('show_keywords')) {
					echo '<br /> <span class="sq-inline-fields-wrapper bottom-margin"><b style="display: inline-block; width: 44px">-'.strtoupper(translate('or')).'-</b> ';
					combo_box($prefix.'_repkeys', $select_options, FALSE, trim($value, '%'), 0, '');
					echo '</span>';
				}
			echo '</div>';

			if ($enable_default) {
				?>
				<script type="text/javascript"><!--
				//<![CDATA[
				handleMetadataDatetimeDefaultClick(document.getElementById('<?php echo $prefix; ?>_default'), '<?php echo $prefix; ?>', <?php echo $default_date_exp; ?>, <?php echo $default_kw_exp; ?>);
				// ]]> -->
				</script>
				<?php
			}

		} else if ($uses_keyword && isset($select_options[trim($value, '%')])) {
			if ($is_default && $enable_default) {
				echo '<span class="sq-metadata-settings-wrapper">';
					if ($is_default) {
						echo ' <em>('.translate('using_default_value').')</em>';
					} else{

					}	
				echo '</span>';
			}
			echo '<i>'.translate('keyword_replacement').': </i>', $select_options[trim($value, '%')];
		} else {
			if ($is_default && $enable_default) {
				echo '<span class="sq-metadata-settings-wrapper">';
					if ($is_default) {
						echo ' <em>('.translate('using_default_value').')</em>';
					} else{

					}	
				echo '</span>';
			}
			echo $field->format();
		}//end if

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if this field has been processed, false if not and it should be ignored
	*
	* @param Metadata_Field_Date	$asset			the asset whose interface we are painting
	* @param string					&$new_value_str	a string by reference that we can store the new values in
	*												set to NULL to revert to default
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Date $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;
		$prefix = $asset->getPrefix();
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			$value = array_get_index($_POST, $prefix.'_repkeys', '');
			if (!empty($value)) {
				$new_value_str = Metadata_Field::encodeValueString('%'.trim($value).'%', Array());
				$processed = TRUE;
			} else if (!empty($_POST[$prefix.'_duration']) && $_POST[$prefix.'_duration'] != '0'  && $_POST[$prefix.'_period'] != '-') {
				$value = date('Y-m-d H:i:s', strtotime($_POST[$prefix.'_duration'].' '.$_POST[$prefix.'_period']));
				$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());
				$processed = TRUE;
			} else {
				$field = $asset->_getDateTimeField($prefix);
				if ($field->processField()) {
					$value = $field->value;
					$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());
					$processed = TRUE;
				}
			}
		}

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param Metadata_Field_Date	$asset		the asset whose interface we are painting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param array					&$tag_value	the metadata tag value
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_Date $asset, $value_str, &$tag_value, Array &$keywords)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$is_default = FALSE;

		if (is_null($value_str)) {
			$value_str = $asset->getDefaultValue();
			$is_default = TRUE;
		}

		$value = '';
		$value_components = $asset->attr('value_components');
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		$original = $value;

		// we create a new array here to pass to our parent so we dont muck up the
		// referenced timestamp values
		if (substr($value, 0, 1) != '%') {
			if (empty($value)) $value = NULL;
			$field = $asset->_getDateTimeField('', $value);
			$value = $field->format();
		}

		ob_start();
			parent::generateMetadata($asset, Metadata_Field::encodeValueString($value, $value_components), $tag_value, $keywords);
			$metadata_tag = ob_get_contents();
		ob_end_clean();

		if (substr($original, 0, 1) == '%') {
			// strip off the last % sign from the end of $key string
			$key = substr($original, 0, -1);

			// prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(' ', '~', $asset->attr('format'));

			// merge the values with '_', add closing % sign and proceed...
			$key = $key.'_'.$format;
			$offset = $asset->attr('offset');
			if (!empty($offset)) {
				$offset = str_replace(' ', '', $offset);
				$key .= '_'.$offset;
			}
			$key .= '%';

			// replace the plain keyword with the one that contains the format
			$tag_value = str_replace($original, $key, $tag_value);

			// replace the plain keyword (or at least it's first occurance in the keywords array)
			// with the one that contains the format
			$keyword = substr($original, 1, -1);
			if (($i = array_search($keyword, $keywords)) !== FALSE) {
				$keywords[$i] = substr($key, 1, -1);
			}

		}// end if

		echo $metadata_tag;

		return TRUE;

	}//end generateMetadata()


}//end class
?>
