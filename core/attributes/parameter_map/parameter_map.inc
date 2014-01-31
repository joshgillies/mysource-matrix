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
* $Id: parameter_map.inc,v 1.34 2012/11/14 23:06:40 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once dirname(dirname(__FILE__)).'/serialise/serialise.inc';

define('SQ_PARAM_MAP_DEFAULT_ATTRIBUTE', 'assetid');
define('SQ_PARAM_MAP_ATTR_OPTION_KEYWORD', 'keyword');
define('SQ_PARAM_MAP_DEFAULT_KEYWORD', 'asset_assetid');

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
* 	Depending on the Source Type the format of the rule array is:
*
* 		*GET, POST
* 			Array(
* 			 'index' 		=> source value
* 			)
*
* 		*ASSET, CURRENT_SITE, USER:
* 			Array(
* 			 'attribute'	=> assetid|SQ_PARAM_MAP_ATTR_OPTION_KEYWORD
* 			 'keyword'		=> the asset keyword // only relevant if 'attribute' = SQ_PARAM_MAP_ATTR_OPTION_KEYWORD
* 			)
*
*		*SET VALUE: A value or global keyword
*		*GLOBALS:	Any Super Global variable like GET, POST, SESSION
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.34 $
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
	* Adds a new parameter to the list of those available
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
	* Moves the parameters from one code to another
	*
	* @param string	$old_code	Old code
	* @param string	$new_code	New code
	*
	* @return void
	* @access public
	*/
	function moveParameter($old_code, $new_code)
	{
		if (!isset($this->_params['parameters'][$old_code])) {
			return;
		}

		if (isset($this->_params['parameters'][$new_code])) {
			trigger_error('Cannot Rename Parameter. Code conflict.', E_USER_WARNING);
			return;
		}


		$this->_params['parameters'][$new_code] = $this->_params['parameters'][$old_code];
		unset($this->_params['parameters'][$old_code]);

		$value = unserialize($this->value);
		if (isset($value[$old_code])) {
			$value[$new_code] = $value[$old_code];
			unset($value[$old_code]);
			$this->value = serialize($value);
		}

	}//end moveParameter()


	/**
	* Removes a parameter from the list
	*
	* @param string	$code	Parameter code
	*
	* @return void
	* @access public
	*/
	function unsetParameter($code)
	{
		$value = unserialize($this->value);
		unset($this->_params['parameters'][$code]);
		unset($value[$code]);
		$this->value = serialize($value);
		$this->processed = $this->setValue($value);

	}//end unsetParameter()


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
		$value = @unserialize($this->value);

		?>

		<table class="sq-backend-table sq-parameter-map">
			<tr>
				<th><?php echo translate('parameter'); ?></th>
				<th><?php echo translate('source'); ?></th>
			</tr>
			<?php

			$source_types = $this->_getSourceTypes();
			if (!empty($value)) {
				foreach ($value as $parameter => $sources) {
					?>
					<tr>
						<td><?php echo $this->_params['parameters'][$parameter]; ?></td>
						<td>
							<table class="sq-backend-table sq-parameter-map-inner">
							<?php
							foreach ($sources as $index => $details) {
								$source_type = $details['source'];
								if (!isset($source_types[$source_type])) continue;

								$one_source_prefix = $prefix.'['.$parameter.']['.$index.']';
								?>
								<tr>
									<th style="width:30ex;"><strong><?php echo $source_types[$source_type]; ?></strong></th>
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
				}//end foreach
			}//end if

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
				$attribute = array_get_index($data, 'attribute', SQ_PARAM_MAP_DEFAULT_ATTRIBUTE);
				$keyword = array_get_index($data, 'keyword', SQ_PARAM_MAP_DEFAULT_KEYWORD);
				if ($read_only) {
					if (is_null($attribute)) {
						echo translate('not_configured');
					} else {
						echo $attribute;
						if ($attribute == SQ_PARAM_MAP_ATTR_OPTION_KEYWORD) {
							echo ' - '.(empty($keyword) ? translate('not_configured') : $keyword);
						}
					}
				} else {
					$options = Array(
								'assetid'							=> translate('asset_id'),
								SQ_PARAM_MAP_ATTR_OPTION_KEYWORD	=> translate('keyword'),
							   );
					combo_box($prefix.'[attribute]', $options, FALSE, $attribute, 0, 'onchange="document.getElementById(\''.$prefix.'[keyword]\').style.visibility = (this.value == \'keyword\') ? \'\' : \'hidden\'"');
					text_box($prefix.'[keyword]', $keyword, '', '', FALSE, ($attribute == SQ_PARAM_MAP_ATTR_OPTION_KEYWORD) ? '' : 'style="visibility: hidden"');
				}
			break;

			case 'SET_VALUE':
				$value = array_get_index($data, 'value');
				if ($read_only) {
					if (is_null($value)) {
						echo translate('not_configured');
					} else {
						echo htmlspecialchars($value);
					}//end if
				} else {
					text_box($prefix.'[value]', $value, 50);
				}//end if
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
					text_box($prefix.'[index]', $value, 50);
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
					if (in_array($fields['source'], Array('ASSET', 'USER', 'CURRENT_SITE'))) {
						unset($value[$index][$j]['index']);

						// clean the keyword and set default value
						if (array_get_index($value[$index][$j], 'attribute', '') == SQ_PARAM_MAP_ATTR_OPTION_KEYWORD) {
							$value[$index][$j]['keyword'] = preg_replace('/(^%|%$)/', '', trim(array_get_index($value[$index][$j], 'keyword', '')));
							if (empty($value[$index][$j]['keyword'])) {
								$value[$index][$j]['keyword'] = SQ_PARAM_MAP_DEFAULT_KEYWORD;
							}
						}
					} else {
						unset($value[$index][$j]['keyword']);
						unset($value[$index][$j]['attribute']);
					}

					if (isset($fields['delete'])) {
						unset($value[$index][$j]);
						if (empty($value[$index])) unset($value[$index]);
					}

				}//end foreach

			}//end foreach ($_POST[$prefix])
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
					if (!empty($rule['index']) && isset($_GET[$rule['index']])) {
						return $_GET[$rule['index']];
					}
				break;

				case 'POST':
					if (!empty($rule['index']) && isset($_POST[$rule['index']])) {
						return $_POST[$rule['index']];
					}
				break;

				// this is for storing data in session
				// note that it is stored in a particular part of global session variable
				case 'SESSION':
					if (!empty($rule['index']) && isset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$rule['index']])) {
						return $_SESSION[SQ_SESSION_SANDBOX_INDEX][$rule['index']];
					}
				break;

				case 'GLOBALS':
					if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$rule['index']]) && (!empty($_SESSION[SQ_SESSION_SANDBOX_INDEX][$rule['index']]))) {
						return $_SESSION[SQ_SESSION_SANDBOX_INDEX][$rule['index']];
					} else if (isset($_POST[$rule['index']]) && (!empty($_POST[$rule['index']]))) {
						return $_POST[$rule['index']];
					} else if (isset($_GET[$rule['index']]) && (!empty($_GET[$rule['index']]))) {
						return $_GET[$rule['index']];
					} else {
						return '';
					}
				break;

				case 'SET_VALUE':
					if (isset($rule['value'])) {
						return replace_global_keywords($rule['value']);
					}//end if
				break;

				case 'ASSET':
				case 'USER':
				case 'CURRENT_SITE':
					$assetid = $this->_getRelevantAssetid($rule['source']);
					if (empty($assetid)) return NULL;
					switch (array_get_index($rule, 'attribute')) {
						case SQ_PARAM_MAP_ATTR_OPTION_KEYWORD:
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							if (!empty($asset)) {
								$keyword = array_get_index($rule, 'keyword', '');
								// default to the assetid if no keyword is defined
								if ($keyword == '') {
									$keyword = SQ_PARAM_MAP_DEFAULT_KEYWORD;
								}
								return $asset->getKeywordReplacement($keyword);
							}
						break;

						case 'assetid':
						default:
							return $assetid;
						break;
					}
				break;

			}//end switch ($rule['source'])

		}//end foreach ($value[$parameter])

		return NULL;

	}//end getParameterValue()


	/**
	* Return the ID of the asset we should work with given the data source specified
	*
	* @param string	$source	ASSET, USER or CURRENT_SITE
	*
	* @return int
	* @access private
	*/
	function _getRelevantAssetid($source)
	{
		$default = NULL;
		switch ($source) {
			case 'ASSET':
				// current asset
				if (SQ_IN_LIMBO) {
					return $_REQUEST['limbo_assetid'];
				} else {
					if (isset($GLOBALS['SQ_SYSTEM']->frontend_asset->id) && !empty($GLOBALS['SQ_SYSTEM']->frontend_asset->id)) {
						return $GLOBALS['SQ_SYSTEM']->frontend_asset->id;
					} else {
						$curr_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
						if (!is_null($curr_asset)) return $curr_asset->id;
						return $default;
					}
				}
			break;

			case 'USER':
				return $GLOBALS['SQ_SYSTEM']->currentUserId();
			break;

			case 'CURRENT_SITE':
				$current_site_id = NULL;
				if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_SITE')) {
					$current_site_id = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_SITE')->id;
				} else {
					if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_ASSET')) {
						$asset_url = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET')->getURL();
						$protocol_pos = strpos($asset_url, '://');
						if ($protocol_pos !== FALSE) {
							$asset_url = substr($asset_url, $protocol_pos + 3);
						}//end if
					} else {
						$asset_url = current_url(FALSE, TRUE);
					}//end if
					$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $asset_url);
					if (isset($asset_lineage[0])) {
						$current_site_id = $asset_lineage[0]['assetid'];
					}//end if
				}//end if
				return $current_site_id;
			break;
		}
		return $default;

	}//end _getRelevantAssetid()


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
				'GET'			=> 'GET '.translate('variable_name'),
				'POST'			=> 'POST '.translate('variable_name'),
				'SESSION'		=> 'SESSION '.translate('variable_name'),
				'ASSET'			=> translate('current_asset'),
				'USER'			=> translate('current_user'),
				'CURRENT_SITE'	=> translate('current_site'),
				'SET_VALUE'		=> translate('set_value'),
				'GLOBALS'		=> translate('super_global'),
			   );

	}//end _getSourceTypes()


}//end class

?>
