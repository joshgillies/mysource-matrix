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
* $Id: design_area_access_history.inc,v 1.11 2008/09/04 02:35:05 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';
define('SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE',	'0');
define('SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT',	'1');
define('SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT',	'2');

/**
* Design_Area_Access_History
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History extends Design_Area
{

	/**
	* Types and names of request variable requirements
	* @var Array
	*/
	public $request_types = Array(
								SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE	=> 'Ignore',
								SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT	=> 'Must exist',
								SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT	=> 'Must NOT exist',
							);


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return string
	* @access public
	*/
	public function printVar($var, Array $args, $value_prefix='', $value_suffix='')
	{
		switch ($var) {
			case 'assetid' :
			case 'asset_link' :
			case 'asset_short_name' :
			case 'asset_name' :
			case 'visit_count' :
				echo '<?php echo htmlspecialchars(array_get_index($asset_info,"'.$var.'")); ?>';
			break;
			case 'access_time' :
			case 'last_visit_time' :
			case 'first_visit_time' :
				echo '<?php echo date("'.$this->attr('time_format').'", htmlspecialchars(array_get_index($asset_info,"'.$var.'"))); ?>';
			break;

			case 'has_update' :
				echo '<?php
					// has_update
					$assetid = array_get_index($asset_info, "assetid");
					if (!empty($assetid)) {
						$last_visit_time = array_get_index($asset_info, "last_visit_time");
						// get asset updated time
						$upasset = $GLOBALS["SQ_SYSTEM"]->am->getAsset($assetid);
						if (!is_null($upasset)) {
							$updated = $upasset->updated;
							if ($last_visit_time < $updated) {
								';
								if (isset($args['label'])) {
									echo 'echo "'.$args['label'].'";';
								}
							echo '
							}
						}
					}
				?>';
			break;

			case 'query_var' :
				echo '<?php
						$url = array_get_index($asset_info,\'asset_link\');
						$position = strpos($url,"?");
						if ($position !== false) {
							$query_string = substr($url, $position+1);
							$matches = Array();
							preg_match_all(\'(([^=^&]+)=([^&]+))is\', $query_string, $matches);
							foreach ($matches[1] as $key => $value) {
								if ($value == \''.$args['name'].'\') {
									echo htmlspecialchars(urldecode($matches[2][$key]));
									break;
								}
							}
						}
				?>';
			break;


			case 'history_count' :
				echo '<?php echo $history_count; ?>';
			break;

			case 'add_to_history_button' :
				$area_id  = $this->attr('id_name');
				$exclude_assets = $this->attr('exclude_assets');
				echo '<?php
						$show_button = FALSE;
						$exclude_assets = '.var_export($exclude_assets,TRUE).';
						$excluded = in_array($ASSET->id, $exclude_assets);
						if (!$excluded) {
							$show_button_for_asset_types = '.var_export($this->attr('add_history_asset_types'), TRUE).';
							if (isset($show_button_for_asset_types[$ASSET->type()])) {
								$show_button = TRUE;
							} else {
								foreach ($show_button_for_asset_types as $type => $inherit) {
									if ($GLOBALS["SQ_SYSTEM"]->am->isTypeDecendant($ASSET->type(), $type)) {
										$show_button = TRUE;
										break;
									}
								}
							}
						}
					?>';
				echo '<?php if ($show_button) { ?>';
				echo '<form action="<?php echo $url; ?>" method="post" id="'.$area_id.'_add_form">';
				echo '<div><input type="hidden" value="add" name="'.$area_id.'_add_to_history" /></div>';
				$submit_str = 'document.getElementById(\''.$area_id.'_add_form\').submit();';
				if (!$this->attr('add_history_button_image')) {
					normal_button($area_id.'_add_to_history_button', $this->attr('add_history_button_label'), $submit_str);
				} else {
					echo '<?php $img_url = ""; ?>';
					$image_id = $this->attr('add_history_image');
					if (!empty($image_id)) {
						echo '<?php
						$img_url = $GLOBALS["SQ_SYSTEM"]->am->getAssetURL('.$image_id.');
						?>';
					}
					echo '<img alt="" id="'.$area_id.'_add_to_history_button" src="<?php echo $img_url ?>" onclick="'.$submit_str.'" onmouseover="this.style.cursor=\'pointer\'" />';
				}
				echo '</form>';
				echo '<?php } ?>';
			break;

			case 'remove_from_history_checkbox' :
				echo '<?php check_box("'.$this->attr('id_name').'_rm_from_history[]", array_get_index($asset_info,\'asset_link\')); ?>';
			break;

			case 'remove_from_history_image' :
				$area_id  = $this->attr('id_name');
				echo '<?php $img_url = ""; ?>';
				$image_id = $this->attr('remove_history_image_small');
					if (!empty($image_id)) {
						echo '<?php
							$img_url = $GLOBALS["SQ_SYSTEM"]->am->getAssetURL('.$image_id.');
						?>';
					}
					echo '<img alt="" id="'.$area_id.'_rm_from_history_<?php echo array_get_index($asset_info,\'assetid\'); ?>" src="<?php echo $img_url ?>" onclick="document.getElementById(\''.$area_id.'_form_remove_asset\').value=\'<?php echo array_get_index($asset_info,\'asset_link\'); ?>\'; document.getElementById(\''.$area_id.'_form\').submit();" onmouseover="this.style.cursor=\'pointer\'" />';
			break;

			case 'remove_from_history_button' :
				$area_id  = $this->attr('id_name');
				if (!$this->attr('remove_history_button_image')) {
					normal_button($area_id.'_remove_from_history_button', $this->attr('remove_history_button_label'), 'document.getElementById(\''.$area_id.'_form\').submit();');
				} else {
					echo '<?php $img_url = ""; ?>';
					$image_id = $this->attr('remove_history_image');
					if (!empty($image_id)) {
						echo '<?php
							$img_url = $GLOBALS["SQ_SYSTEM"]->am->getAssetURL('.$image_id.');
						?>';
					}
					echo '<img name="'.$area_id.'_remove_from_history_button" src="<?php echo $img_url ?>" onclick="document.getElementById(\''.$area_id.'_form\').submit();" onmouseover="this.style.cursor=\'pointer\'" />';
				}
			break;

			default :
				parent::printVar($var, $args);

		}//end switch

	}//end printVar()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$element	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $element)
	{
		$area_id = $this->attr('id_name');

		switch (array_get_index($element, 'operation')) {
			case 'asset' :
				echo '<?php if (!isset($url)) {
					$url = current_url();
				}
				include_once(SQ_SYSTEM_ROOT."/core/lib/html_form/html_form.inc");
				?>';
				echo '
				<form action="<?php echo $url; ?>" method="post" id="'.$area_id.'_form">
				<div><input type="hidden" id="'.$area_id.'_form_remove_asset" name="'.$area_id.'_form_remove_asset" value="" /></div>

					<?php
						foreach($history as $position => $asset_info) {
					?>
				';
				if ($asset_contents = $this->attr('asset_contents')) {
					foreach ($asset_contents as $asset_content) {
						$this->_printContentItemCode($asset_content);
					}
				}
				echo '
					<?php
						}//end foreach
					?>
					</form>
				';
				return TRUE;
			default :
				return parent::_printContentItemCode($element);
		}//end switch

	}//end _printContentItemCode()


	/**
	* Return whether this DA has been given sufficient details to be able to print stuff
	*
	* This function can be overriden by children to enforce required attributes etc
	*
	* @return boolean
	* @access protected
	*/
	protected function _canInit()
	{
		return count($this->attr('asset_types')) && count($this->attr('asset_count'));

	}//end _canInit()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		$area_id = $this->attr('id_name');
		$exclude_assets = $this->attr('exclude_assets');
		$this_types = $this->attr('asset_types');
		$max_count = $this->attr('asset_count');
		$time_format = $this->attr('time_format');
		$auto_access_history = $this->attr('auto_access_history');

		echo '
			<?php
				$exclude_assets = '.var_export($exclude_assets,TRUE).';
				$allowed_asset_types = '.var_export($this_types,TRUE).';
				$max_asset_count = '.$max_count.';
				//// Access History Design Area

				// initialise the history data
				if (!isset($_SESSION["design_areas"]["'.$area_id.'"])) {
					$_SESSION["design_areas"]["'.$area_id.'"] = Array();
				}
				$saved_data = $_SESSION["design_areas"]["'.$area_id.'"];
				$history = array_get_index($saved_data, "history", Array());
				$history_hash = array_get_index($saved_data, "history_hash", Array());

				// check if this asset type is being tracked
				$require_query_vars = array_get_index($allowed_asset_types, $ASSET->type());

				$excluded = in_array($ASSET->id, $exclude_assets);
				if (!is_null($require_query_vars) && (!$excluded)) {
					// check how existence of query vars influences history recording
					$url = null;
					$query_var = array_get_index($_SERVER, "QUERY_STRING");
					if (!empty($query_var)) {
						$query = TRUE;
						$query_var = "?".$query_var;
					} else {
						$query = false;
					}
					$current_url = current_url();

					if ((($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT.') && $query) || ($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE.')) {
						$url = $current_url.$query_var;
					} else if (($require_query_vars == '.SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT.') && !$query) {
						$url = $current_url;
					}

					// now update history if necessary
					if (!is_null($url)){
						';
						if (!$auto_access_history) {
							echo 'if (isset($_REQUEST["'.$area_id.'_add_to_history"])) {';
						}
						echo '
							// if this url already exists in history, remove it
							$history_index = array_get_index($history_hash, $url);

							$visit_count = 1;
							$first_access_time = time();
							if (!is_null($history_index)) {
								$visit_count = array_get_index($history[$history_index], "visit_count", 0);
								$first_access_time = array_get_index($history[$history_index], "first_visit_time", $first_access_time);
								unset($history[$history_index]);
							}

							if (count($history) > $max_asset_count) {
								array_pop($history);
							}

							// create a new history entry
							$new_entry["assetid"] 			= $ASSET->id;
							$new_entry["asset_link"] 		= $url;
							$new_entry["asset_name"] 		= $ASSET->name;
							$new_entry["asset_short_name"]	= $ASSET->short_name;
							$new_entry["access_time"]		= time();
							$new_entry["visit_count"]		= $visit_count;
							$new_entry["first_visit_time"]	= $first_access_time;
							$new_entry["last_visit_time"]	= time();

							array_unshift($history, $new_entry);
							';
						if (!$auto_access_history) echo '}';

						echo '
						}//end if (!is_null(url))
					}//end if (!is_null(require_query_vars))


					// if current page is in history update its details like visit count, access time etc
					$history_index = array_get_index($history_hash, current_url(), NULL);

					if (!is_null($history_index)) {
						$history[$history_index]["visit_count"] = array_get_index($history[$history_index], "visit_count", 0) + 1;
						$history[$history_index]["last_visit_time"] = time();
					}

					// remove selected assets from history
					if (isset($_REQUEST["'.$area_id.'_rm_from_history"]) && !empty($_REQUEST["'.$area_id.'_rm_from_history"])) {

						foreach ($_REQUEST["'.$area_id.'_rm_from_history"] as $remove_url) {
							$history_index = array_get_index($history_hash, urldecode($remove_url));

							if (!is_null($history_index)) {
								unset($history[$history_index]);
							}
						}
					}

					if (isset($_REQUEST["'.$area_id.'_form_remove_asset"]) && !empty($_REQUEST["'.$area_id.'_form_remove_asset"])) {
						$remove_url = $_REQUEST["'.$area_id.'_form_remove_asset"];
						$history_index = array_get_index($history_hash, urldecode($remove_url));
						if (!is_null($history_index)) {
							unset($history[$history_index]);
						}
					}

					// update history hash
					$new_history_hash = Array();
					foreach ($history as $index => $contents) {
						if (isset($contents["asset_link"])) {
							$new_history_hash[$contents["asset_link"]] = $index;
						}
					}


					$_SESSION["design_areas"]["'.$area_id.'"]["history"] = $history;
					$_SESSION["design_areas"]["'.$area_id.'"]["history_hash"] = $new_history_hash;
					';

					if ($auto_access_history) {
						echo '
						// now that the session is saved, we want to remove the first item because
						// we do not want to list the current page
						array_shift($history);
						';
					}
					echo '
				$history = array_slice($history, 0, $max_asset_count);
				$history_count = count($history);
			?>
		';

	}//end _printInitCode()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that are not mentioned in the parse file are normally set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from that process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'asset_contents';
		return $res;

	}//end getProtectedAttrs()

        
        
        
        
    /**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value)
	{
		 
		//parse attr such as asset_types, add_history_types 
		// For asset_types, e.g folder[1]  means asset type is foler, parameters must exists.  
		//The digit means the parameters requirements , 0=> ignore, 1=> must exists,2=>must not exists 
		//For add_history_asset_types , e.g trigger_action_add_url[0] means Add URL, not inherit.
		//Digit means inherit or not

		if (($name == 'asset_types' || $name == 'add_history_asset_types') && is_scalar($value)) {
			$value = preg_split('/[ ,]+/', (string)$value);
			$processed_value = Array();

			foreach ($value as $type_content) {
				preg_match('/(?<type_code>\w+)(\[(?<parameters>[012])\])?/', $type_content, $matches);

				if (!$GLOBALS['SQ_SYSTEM']->am->installed($matches['type_code'])) {
					trigger_localised_error('CORE0145', E_USER_WARNING, $matches['type_code']);
					return FALSE;
				}
				if (empty($matches['parameters'])){
					$matches['parameters'] = 0;
				}
				$processed_value[$matches['type_code']] = $matches['parameters'];	
			}
			$value = $processed_value;
		}

		else if ($name == 'exclude_assets' && is_scalar($value)) {
			$value = preg_split('/[ ,]+/', (string)$value);
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

}//end class
?>
