<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: parameter_map.inc,v 1.14 2006/09/13 00:17:14 colivar Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once dirname(dirname(__FILE__)).'/serialise/serialise.inc';

/**
* Parameter_Map asset attribute
*
*   Attributes of this type store and allow for the manipulation of mappings
*   from runtime variable values (GET, POST etc) to certain parameters of
*   an asset.  When declaring the attribute in the asset's management class
*   you need to pass a parameter called 'parameters', which is a
*   code => full_name array of the parameters you want to make mappable.
*   When requesting the runtime value for a parameter you give its code.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Parameter_Map extends Asset_Attribute_Serialise
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Parameter_Map($attributeid=0, $value=NULL)
	{
		if (!isset($this->_params['parameters'])) {
			$this->_params['parameters'] = Array();
		}
		if ($value == NULL) $value = Array();
		$this->Asset_Attribute_Serialise($attributeid, $value);

		// TODO: Investigate efficiency of unserialising once vs. every time the value is needed.

	}//end constructor


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$code	Parameter code
	* @param string	$label	Friendly name describing the parameter
	*
	* @return void
	* @access public
	*/
	function setParameter($code, $label)
	{
		$this->_params['parameters'][$code] = $label;

	}//end setParameter()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$value = unserialize($this->value);

		?>

		<table border="0" cellpadding="0" cellspacing="0" class="sq-backend-table">
			<tr>
				<th><?php echo translate('parameter'); ?></th>
				<th><?php echo translate('source'); ?></th>
			</tr>
			<?php

			$source_types = $this->_getSourceTypes();
			foreach ($value as $parameter => $sources) {
				?>
				<tr>
					<td><?php echo $this->_params['parameters'][$parameter]; ?></td>
					<td>
						<table border="0" cellpadding="5" cellspacing="0" class="sq-backend-table">
						<?php
						foreach ($sources as $index => $details) {
							$source_type = $details['source'];
							$one_source_prefix = $prefix.'['.$parameter.']['.$index.']';
							?>
							<tr>
								<td style="width:30ex"><strong><?php echo $source_types[$source_type]; ?></strong></td>
								<td><?php $this->_printSourceInterface($one_source_prefix, $details, $read_only); ?></td>
								<td style="text-align:right">
									<?php
									if (!$read_only) {
										check_box($one_source_prefix.'[delete]');
										echo '<label for="'.$one_source_prefix.'[delete]">'.translate('delete').'</label>';
									}
									?>
								</td>
							</tr>
							<?php
						}
						?>
						</table>
					</td>
				</tr>
				<?php
			}
			if (!$read_only) {
				?>
				<tr>
					<td><?php $this->_printParameterChooser($prefix.'[new][0]'); ?></td>
					<td><?php $this->_printNewSourceChooser($prefix.'[new][0]'); ?></td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

	}//end paint()


	/**
	* Paint the chooser for a new source
	*
	* @param string	$prefix	The prefix for the fields' names
	*
	* @return void
	* @access private
	*/
	function _printNewSourceChooser($prefix)
	{
		combo_box($prefix.'[source]', $this->_getSourceTypes(), FALSE, Array());

	}//end _printNewSourceChooser()


	/**
	* Paint the fields to choose a data source
	*
	* @param string	$prefix		The prefix for the fields' names
	* @param string	$data		The current value for the source-type field
	* @param string	$read_only	True if the interface to be printed is read-only
	*
	* @return void
	* @access private
	*/
	function _printSourceInterface($prefix, $data, $read_only=TRUE)
	{
		$source_type = array_get_index($data, 'source');

		if (!$read_only) {
			hidden_field($prefix.'[source]', $source_type);
		}

		switch ($source_type) {
			case 'ASSET':
			case 'CURRENT_SITE':
			case 'USER':
				$attribute = array_get_index($data, 'attribute');
				if ($read_only) {
					if (is_null($attribute)) {
						echo translate('not_configured');
					} else {
						echo $attribute;
					}
				} else {
					combo_box($prefix.'[attribute]', $this->_getAssetAttributeOptions(), FALSE, Array($attribute));
				}
			break;

			case 'ASSET_QUESTION':
				$asset_id = array_get_index($data, 'index');
				if ($read_only) {
					if (is_null($asset_id)) {
						echo translate('not_configured');
					} else {
						echo get_asset_tag_line($asset_id, 'details');
					}
				} else {
					asset_finder($prefix.'[asset_finder]', $asset_id, Array('form_question' => 'D'));
				}
			break;

			default:
				$value = array_get_index($data, 'index');
				if ($read_only) {
					if (is_null($value)) {
						echo translate('not_configured');
					} else {
						echo $value;
					}
				} else {
					text_box($prefix.'[index]', $value);
				}


		}//end switch

	}//end _printSourceInterface()


	/**
	* Paints the field to choose a parameter for a new mapping
	*
	* @param string	$prefix	The prefix for the field's name
	*
	* @return void
	* @access private
	*/
	function _printParameterChooser($prefix)
	{
		combo_box($prefix.'[parameter]', Array(''=>'-- '.translate('new_mapping').' --')+$this->_params['parameters'], FALSE, Array());

	}//end _printParameterChooser()


	/**
	* Processes the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$value = Array();
		if (isset($_POST[$prefix]) && is_array($_POST[$prefix])) {
			foreach ($_POST[$prefix] as $index => $details) {
				if ($index == 'new') continue;
				$value[$index] = $details;
				foreach ($details as $j => $fields) {
					if ($fields['source'] == 'ASSET' || $fields['source'] == 'USER' || $fields['source'] == 'CURRENT_SITE') {
						unset($value[$index][$j]['index']);
					} else if ($fields['source'] == 'ASSET_QUESTION') {
						unset($value[$index][$j]['attribute']);
						$value[$index][$j]['index'] = $fields['asset_finder']['assetid'];
						unset($value[$index][$j]['asset_finder']);
					} else {
						unset($value[$index][$j]['attribute']);
					}

					if (isset($fields['delete'])) {
						unset($value[$index][$j]);
						if (empty($value[$index])) unset($value[$index]);
					}

				}
			}

			foreach ($_POST[$prefix]['new'] as $new_mapping) {
				if (empty($new_mapping['parameter'])) continue;

				$param = $new_mapping['parameter'];
				unset($new_mapping['parameter']);

				$value[$param][] = $new_mapping;
			}

			if (!$this->validateValue($value)) return FALSE;

			$this->processed = $this->setValue($value);

		}//end if (is_array())

	}//end process()


	/**
	* Get the runtime value for the specified parameter, returns NULL is no value found
	*
	* @param string	$parameter	The code of the parameter we want the value for
	*
	* @return mixed NULL|object
	* @access public
	*/
	function getParameterValue($parameter)
	{
		// TODO: Check why we need to unserialize every time (see constructor)
		$value = unserialize($this->value);

		if (!isset($value[$parameter])) return NULL;
		foreach ($value[$parameter] as $rule) {
			switch ($rule['source']) {
				case 'GET':
					if (isset($_GET[$rule['index']])) {
						return $_GET[$rule['index']];
					}
				break;

				case 'POST':
					if (isset($_POST[$rule['index']])) {
						return $_POST[$rule['index']];
					}
				break;

				case 'ASSET':
					switch ($rule['attribute']) {
						case 'assetid':
							return (SQ_IN_LIMBO) ? $_REQUEST['limbo_assetid'] : $GLOBALS['SQ_SYSTEM']->frontend_asset->id;
						break;
						case 'name':
						case 'short_name':
							if (SQ_IN_LIMBO) {
								$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_REQUEST['limbo_assetid']);
								return $asset->attr($rule['attribute']);
							} else {
								return $GLOBALS['SQ_SYSTEM']->frontend_asset->attr($rule['attribute']);
							}
						break;
					}
				break;

				case 'USER':
					switch ($rule['attribute']) {
						case 'assetid':
							return $GLOBALS['SQ_SYSTEM']->currentUserId();
						break;
						case 'name':
						case 'short_name':
							return $GLOBALS['SQ_SYSTEM']->user->attr($rule['attribute']);
						break;
					}

				// this is primitive, but allows for great flexibility
				case 'STATIC':
					// need to restructure the rules array to be more logical
					return $rule['index'];

				break;


				// NOTE: This might need to be replaced by a general asset handler
				case 'ASSET_QUESTION':
					// question id is stored in the index
					$asset_id = $rule['index'];

					$question_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);

					// TODO: (maybe?) Check that the form has been processed
					$value = $question_asset->getValue();
					return $value;

				break;

				case 'CURRENT_SITE':
					// if the attribute is not set we use assetid by default
					if (!isset($rule['attribute'])) {
						$rule['attribute'] = 'assetid';
					}
					switch ($rule['attribute']) {
						case 'assetid':
							$current_site_id = NULL;
							$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL();
							if (isset($asset_lineage[0])) {
								$current_site_id = $asset_lineage[0]['assetid'];
								return $current_site_id;
							}
						break;
					}
					return NULL;
				break;
			}//end switch ($rule['source'])

		}//end foreach ($value[$parameter])

		return NULL;

	}//end getParameterValue()


	/**
	* Returns the list of parameters available
	*
	* @return array
	* @access public
	*/
	function getParameters()
	{
		return array_keys(unserialize($this->value));

	}//end getParameters()


	/**
	* Get the list of source types - the list of sources to draw from
	*
	* @return array
	* @access private
	*/
	function _getSourceTypes()
	{
		// TODO: Implement Generic Asset Chooser
		// Should allow any asset to be selected and data to be extracted from it based on what the asset makes available
		// 'ASSET_ANY'		=> 'Any Asset (Paint Asset Chooser)',

		return Array(
				'GET'				=> 'GET '.translate('variable_name'),
				'POST'				=> 'POST '.translate('variable_name'),
				'ASSET'				=> translate('current_asset'),
				'USER'				=> translate('current_user'),
				'CURRENT_SITE'		=> translate('current_site'),
				'ASSET_QUESTION'	=> translate('form_question_asset'),
				'STATIC'			=> translate('static_value'),
			   );

	}//end _getSourceTypes()


	/**
	* Return the list of options available under the ASSET source type
	*
	* @return array
	* @access private
	*/
	function _getAssetAttributeOptions()
	{
		$options = Array(
					'assetid'	=> translate('asset_id'),
				   );

		return $options;

	}//end _getAssetAttributeOptions()


}//end class

?>
