<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml_edit_fns.inc,v 1.5.2.1 2009/11/30 04:10:38 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_Xml_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.5.2.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Xml_Edit_Fns extends Data_Source_Edit_Fns
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
	* Paints the interface for entering XML feed details
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFeed(&$asset, &$o, $prefix)
	{
		$url = html_entity_decode($asset->attr('url'));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box($prefix.'feed_url', $url, 30);
			return TRUE;
		} else {
			echo $url;
		}
		return FALSE;

	}//end paintFeed()


	/**
	* Processes the interface for entering XML feed details
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processFeed(&$asset, &$o, $prefix)
	{
		$url = htmlentities($asset->attr('url'));
		if (isset($_POST[$prefix.'feed_url'])) {
			$link = htmlentities(trim($_POST[$prefix.'feed_url']));
			if (strcasecmp($url, $link) != 0) {
				$asset->setAttrValue('url', $link);
				return TRUE;
			}
		}
		return FALSE;

	}//end processFeed()


	/**
	* Paints the interface for entering comma-separated tag names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintTags(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_area($prefix.'tag_names', !empty($tags) ? implode(',', $tags) : '', 40);
			return TRUE;
		} else {
			echo !empty($tags) ? implode(',', $tags) : '';
		}
		return FALSE;

	}//end paintTags()


	/**
	* Processes the interface for entering comma-separated tag names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processTags(&$asset, &$o, $prefix)
	{
		$current_tags = htmlentities($asset->attr('tags'));
		if (isset($_POST[$prefix.'tag_names'])) {
			$posted_tags = htmlentities($_POST[$prefix.'tag_names']);
			if (strcasecmp($current_tags, $posted_tags) != 0) {
				$exploded = explode(',', $posted_tags);
				// trim excess whitespace for accuracy
				array_walk($exploded, Array(&$this, '_trimValue'));
				$asset->setAttrValue('tags', serialize($exploded));
				return TRUE;
			}
		}
		return FALSE;

	}//end processTags()


	/**
	* Paints the interface for entering comma-separated tag attribute names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintTagAttributes(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$tag_attributes = unserialize(html_entity_decode($asset->attr('tag_attributes')));
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('tag');?></th>
				<th><?php echo translate('attributes');?></th>
			</tr>
			<?php
			for ($i=0; $i<count($tags); $i++) {
				?>
				<tr>
					<td><?php echo $tags[$i];?> </td>
					<?php if ($wa) { ?>
						<td><?php text_box($prefix.'tagattr_'.$i, isset($tag_attributes[$tags[$i]]) ? implode(',', $tag_attributes[$tags[$i]]) : '', 30); ?> </td>
					<?php } else { ?>
						<td><?php echo isset($tag_attributes[$tags[$i]]) ? implode(',', $tag_attributes[$tags[$i]]) : ''; ?> </td>
					<?php } ?>
				</tr>
			<?php
			}
		?>
		</table>
		<?php

		return $wa;

	}//end paintTagAttributes()


	/**
	* Processes the interface for entering comma-separated tag attribute names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processTagAttributes(&$asset, &$o, $prefix)
	{
		$i = 0;
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$results = Array();
		foreach ($_POST as $tag => $attributes) {
			if ($tag === $prefix.'tagattr_'.$i) {
					$exploded = explode(',', htmlentities($attributes));
					// trim excess whitespace for accuracy
					array_walk($exploded, Array(&$this, '_trimValue'));
					if (isset($tags[$i])) $results[$tags[$i]] = $exploded;
				$i++;
			}
		}
		if (!empty($results)) {
			$asset->setAttrValue('tag_attributes', serialize($results));
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processTagAttributes()


	/**
	* Trims string by reference
	*
	* @param string	&$value	value to trim
	*
	* @return void
	* @access private
	*/
	function _trimValue(&$value)
	{
		$value = trim($value);

	}//end _trimValue()

	/**
	* Paint the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function paintDynamicData(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$var_list = $asset->attr('dynamic_vars');

		// MAYBE: detect keywords in SQL and inject them into variables

		$o->openRaw();

		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<th>Name</th>';
		echo '<th>Default Value</th>';
		if ($write_access) {
			echo '<th width="10%" align="right" style="border-left: dotted 1px grey;">Delete</th>';
		}
		echo '</tr>';
		foreach ($var_list as $var => $value) {
			$var_id = base64_encode($var);

			echo '<tr>';
			echo '<td>';
			if ($write_access) {
				text_box($prefix.'_var_name['.$var_id.']', urlencode($var));
			} else {
				echo urlencode($var);
			}
			echo '</td>';
			echo '<td valign="top" align="left">';
			if ($write_access) {
				text_box($prefix.'_var_value['.$var_id.']', urlencode($value));
			} else {
				echo urlencode($value);
			}
			echo '</td>';
			if ($write_access) {
				echo '<td style="border-left: dotted 1px grey;">';
				check_box($prefix.'_delete_var['.$var_id.']');
				echo '</td>';
			}
			echo '</tr>';
		}

		if ($write_access) {
			echo '<tr class="new-data-row">';
			echo '<th colspan="3" align="left">';
			echo '<em style="color:#666">Add New Variables Below</em>';
			echo '</th>';
			echo '</tr>';

			echo '<tr class="new-data-row">';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_name[]', NULL);
			echo '</td>';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_value[]', NULL);
			echo '</td>';
			echo '<td style="border-left: dotted 1px grey;">';
			echo '</td>';
			echo '</tr>';
		}
		echo '</table>';

		$o->closeRaw();

		$o->sectionNote(translate('db_data_source_variable_section_note'));

		$o->closeSection();

		$o->openSection('Data Mappings');
		$o->openRaw();
		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();

		foreach ($var_list as $var_name => $default) {
			$map->setParameter($var_name, 'Variable: '.$var_name);
		}

		$map->paint($prefix.'_parameter_map', !$write_access);
		$o->closeRaw();

		$o->sectionNote(translate('db_data_source_mappings_section_note'));

		return $write_access;

	}//end paintDynamicData()


	/**
	* Process the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function processDynamicData(&$asset, &$o, $prefix)
	{
		$var_name_pattern = '/^[a-zA-Z_\-0-9\.]+$/';


		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;

		$to_delete = array_get_index($_REQUEST, $prefix.'_delete_var', Array());
		$var_value = array_get_index($_REQUEST, $prefix.'_var_value', Array());
		$var_name = array_get_index($_REQUEST, $prefix.'_var_name', Array());

		$var_list = $asset->attr('dynamic_vars');

		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();
		foreach ($var_list as $name => $default) {
			$map->setParameter($name, 'Variable: '.$name);
		}

		$map->process($prefix.'_parameter_map');

		foreach ($var_value as $hash => $value) {
			$name = base64_decode($hash);
			if (isset($to_delete[$hash])) {
				unset($var_list[$name]);
				// also need to delete it from the PARAMETER MAP
				if (in_array($name, $parameter_names)) {
					$map->unsetParameter($name);
				}

				continue;
			}

			$var_list[$name] = $value;
		}

		$new_var_list = Array();
		$rename_success = FALSE;

		// now we rename variables
		foreach ($var_name as $hash => $new_name) {
			$new_name = trim($new_name);

			if (empty($new_name)) {
				$rename_success = FALSE;
				trigger_error('Variable name cannot be empty. No variables were renamed.', E_USER_NOTICE);
				break;
			}

			// check new name against regex
			$new_name_is_valid = preg_match($var_name_pattern, $new_name);
			if (!$new_name_is_valid) {
				$rename_success = FALSE;
				trigger_error('Variable name ['.htmlentities($new_name).'] contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
				break;
			}

			$name = base64_decode($hash);

			// skip the deleted vars
			if (!isset($var_list[$name])) continue;

			if (isset($new_var_list[$new_name])) {
				$rename_success = FALSE;
				trigger_error('Could not rename variables because variable name "'.htmlentities($new_name).'" already exists', E_USER_NOTICE);
				break;
			}

			if ($new_name != $name) {
				if (in_array($name, $parameter_names)) {
					$map->moveParameter($name, $new_name);
				}
				$rename_success = TRUE;
			}

			$new_var_list[$new_name] = $var_list[$name];

		}//end foreach


		if ($rename_success) $var_list = $new_var_list;

		$new_names = array_get_index($_REQUEST, $prefix.'_new_dynamic_data_name', Array());
		if (!empty($new_names)) {

			foreach ($new_names as $seq_number => $new_var_name) {
				$new_var_name = trim($new_var_name);

				$new_var_value = array_get_index($_REQUEST[$prefix.'_new_dynamic_data_value'], $seq_number, '');
				if (empty($new_var_name)) {
					if (!empty($new_var_value)) {
						trigger_error('New variable name cannot be empty', E_USER_WARNING);
					}
					continue;
				}

				// check new name against regex
				$new_name_is_valid = preg_match($var_name_pattern, $new_var_name);
				if (!$new_name_is_valid) {
					trigger_error('Variable name "'.htmlentities($new_var_name).'" contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
					continue;
				}

				if (isset($var_list[$new_var_name])) {
					trigger_error('Variable Name "'.htmlentities($new_var_name).'" is already in use', E_USER_WARNING);
					continue;
				}

				$var_list[$new_var_name] = $new_var_value;
				$map->setParameter($new_var_name, 'Variable: '.$new_var_name);
			}
		}

		$asset->setAttrValue('dynamic_vars', $var_list);
		$asset->setAttrValue('parameter_map', unserialize($map->value));

		return TRUE;

	}//end processDynamicData()


	/**
	* Paints the interface for entering comma-separated tag names to be enlosed in CDATA
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintCDATATags(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('cdata_tags')));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_area($prefix.'cdata_tag_names', !empty($tags) ? implode(',', $tags) : '', 40);
			return TRUE;
		} else {
			echo !empty($tags) ? implode(',', $tags) : '';
		}
		return FALSE;

	}//end paintTags()


	/**
	* Process the interface for entering comma-separated tag names to be enlosed in CDATA
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processCDATATags(&$asset, &$o, $prefix)
	{
		$current_tags = htmlentities($asset->attr('cdata_tags'));
		if (isset($_POST[$prefix.'cdata_tag_names'])) {
			$posted_tags = htmlentities($_POST[$prefix.'cdata_tag_names']);

			if (strcasecmp($current_tags, $posted_tags) != 0) {
				$exploded = explode(',', $posted_tags);
				// trim excess whitespace for accuracy
				array_walk($exploded, Array(&$this, '_trimValue'));
				$asset->setAttrValue('cdata_tags', serialize($exploded));
				// overwrite the cache with this new XML feed url
				$asset->setResultSet(Array(), $asset->attr('url'));
				$asset->getResultSet($asset->attr('url'));
				return TRUE;
			}
		}
		return FALSE;

	}//end processTags()


}//end class

?>
