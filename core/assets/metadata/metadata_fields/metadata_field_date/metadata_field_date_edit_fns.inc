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
* $Id: metadata_field_date_edit_fns.inc,v 1.34 2007/01/29 04:03:46 tbarrett Exp $
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
* @version $Revision: 1.34 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Date_Edit_Fns()
	{
		$this->Metadata_Field_Edit_Fns();

	}//end Metadata_Field_Date_Edit_Fns()


	/**
	* Paint the interface for viewing available date format keys
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDateFormatKeys(&$asset, &$o, $prefix)
	{
		ob_start();
			?><a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field_date/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=640,height=520'); return false;"><?php echo translate('click_here'); ?></a><?php
			$replacements_link = ob_get_contents();

		ob_end_clean();

		$o->note(translate('metadata_date_object_keys', $replacements_link));

	}//end paintDateFormatKeys()


	/**
	* Processes the interface for viewing date format keys
	*
	* Note that keyword instructions are not processed so this function always
	* returns false
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDateFormatKeys(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processDateFormatKeys()


	/**
	* Print the interface for editing the default date
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultDateTime(&$asset, &$o, $prefix)
	{
		$default = $asset->attr('default');

		$uses_keyword = (substr($default, 0, 1) == '%');

		// parsing the keyword if its not a datestamp already before proceeding...
		if ($uses_keyword) {
			$temp_val = str_replace('%', '', $default);
			$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			$res = $mm->generateKeywordReplacements($asset, Array($temp_val), FALSE); // just return value
			$default = $res[$temp_val];
		}

		if (empty($default)) $default = NULL;
		$field = $asset->_getDateTimeField($prefix, $default);

		if ($asset->writeAccess('attributes')) {
			$field->printField();
		} else {
			if ($uses_keyword) {
				?><em><?php echo translate('metadata_field_date_keyword_in_use'); ?></em><?php
			} else {
				echo $field->format();
			}
		}

	}//end paintDefaultDateTime()


	/**
	* Processes the interface for editing the default date
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefaultDateTime(&$asset, &$o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		if ($asset->writeAccess('attributes') && $field->processField()) {
			return $asset->setAttrValue('default', $field->value);
		}

		return FALSE;

	}//end processDefaultDateTime()


	/**
	* Print the interface for editing the time offset
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintOffset(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processOffset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_offset'])) return FALSE;
		return $asset->setAttrValue('offset', $_POST[$prefix.'_offset']);

	}//end processOffset()


	/**
	* Paint the interface for dropdown selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRepKeys(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$default = $asset->attr('default');

		$select_options = Array(
							''					=> '-- '.strtolower(translate('keywords')).' --',
							'%asset_created%'	=> translate('creation_date'),
							'%asset_updated%'	=> translate('last_updated_date'),
							'%asset_published%'	=> translate('last_published_date'),
						  );

		// displaying default value if any values were actually saved in the array
		if ($write_access) {
			combo_box($prefix.'_repkeys', $select_options, FALSE, $default);
		} else {
			if (strpos($default, '%') === 0) {
				echo $select_options[$default];
			} else {
				echo translate('none_selected');
			}
		}

		$o->note(translate('metadata_use_selected_replacements'));

		return TRUE;

	}//end paintRepKeys()


	/**
	* Process the interface for dropdown selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRepKeys(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$new_default = trim($_POST[$prefix.'_repkeys']);
		if ($new_default != '') {
			$asset->setAttrValue('default', $new_default);
		}

		return TRUE;

	}//end processRepKeys()


	/**
	* Print the interface for editing how the editing interface parameters for the date time field that is used
	* to enter the metadata value for an individual asset
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSetupParams(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSetupParams(&$asset, &$o, $prefix)
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
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	does the current user have write access to the asset
	*									that we are setting metadata for (NOT THE SAME AS $asset)
	* @param boolean	$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(&$asset, &$o, $value_str, $write_access, $enable_default=TRUE)
	{
		$prefix = $asset->getPrefix();

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
							'%asset_created%'	=> translate('creation_date'),
							'%asset_updated%'	=> translate('last_updated_date'),
							'%asset_published%'	=> translate('last_published_date'),
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
		if ($write_access && $asset->attr('editable')) {

			$field->printField();
			if ($asset->attr('show_keywords')) {
				echo ' <b>-'.strtoupper(translate('or')).'-</b> ';
				combo_box($prefix.'_repkeys', $select_options, FALSE, $value, 0, '');
			}

			if ($enable_default) {
				echo ' <b>-'.strtoupper(translate('or')).'-</b> ';
				if ((substr($default_val, 0, 1) == '%') || empty($value)) {
					$default_date_exp = 'null';
					$default_kw_exp = "'$default_val'";
				} else {
					list($dy, $dm, $dd, $dh, $di, $ds) = sscanf($default_val, '%d-%d-%d %d:%d:%d');
					$default_date_exp = "{y:$dy, m:$dm, d:$dd, h:$dh, i:$di, s:$ds}";
					$default_kw_exp = 'null';
				}

				check_box($prefix.'_default', '1', $is_default, 'handleMetadataDatetimeDefaultClick(this, \''.$prefix.'\', '.$default_date_exp.', '.$default_kw_exp.')');
				label(translate('use_default'), $prefix.'_default');
				?>
				<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/metadata_field_date/js/metadata_field_date.js"></script>
				<script type="text/javascript"><!--
				//<![CDATA[
				handleMetadataDatetimeDefaultClick(document.getElementById('<?php echo $prefix; ?>_default'), '<?php echo $prefix; ?>', <?php echo $default_date_exp; ?>, <?php echo $default_kw_exp; ?>);
				// ]]> -->
				</script>
				<?php
			}
		} else if ($uses_keyword && isset($select_options[$value])) {
			echo '<i>'.translate('keyword_replacement').': </i>', $select_options[$value];
			if ($is_default && $enable_default) {
				echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
			}
		} else {
			echo $field->format();
			if ($is_default && $enable_default) {
				echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
			}
		}//end if

		return TRUE;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object	&$asset			the asset whose interface we are processing
	* @param string	&$new_value_str	a string by reference that we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_value_str)
	{
		// nothing for us to do
		if (!$asset->attr('editable')) return TRUE;

		$prefix = $asset->getPrefix();

		$value = array_get_index($_POST, $prefix.'_repkeys', '');
		$use_default = array_get_index($_POST, $prefix.'_default', '');

		if ($use_default) {
			$new_value_str = NULL;
		} else {
			// if we aren't using keywords...get the date that was set
			if ($value == '') {
				$field = $asset->_getDateTimeField($prefix);
				if ($field->processField()) {
					$value = $field->value;
				} else {
					// field could not be processed
					trigger_localised_error('CORE0203', E_USER_WARNING, $asset->name);
					return FALSE;
				}
			}
		}

		if (!is_null($new_value_str)) {
			$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());
		}

		return TRUE;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	* @param array	&$tag_value	the metadata tag value
	* @param array	&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $value_str, &$tag_value, &$keywords)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything
		if ($frontend_option == 'hide') return TRUE;

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