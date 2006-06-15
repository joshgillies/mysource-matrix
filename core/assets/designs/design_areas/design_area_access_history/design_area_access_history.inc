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
* $Id: design_area_access_history.inc,v 1.4.2.1 2006/06/15 01:24:45 tbarrett Exp $
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
* @version $Revision: 1.4.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History extends Design_Area
{

	/**
	* @var Array	$request_types	Types and names of request variable requirements
	*/
	var $request_types = Array(
							SQ_DESIGN_AREA_HISTORY_STATUS_IGNORE	=> 'Ignore',
							SQ_DESIGN_AREA_HISTORY_STATUS_PRESENT	=> 'Must exist',
							SQ_DESIGN_AREA_HISTORY_STATUS_ABSENT	=> 'Must NOT exist',
						 );


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		switch ($var) {
			case 'assetid' :
			case 'asset_link' :
			case 'asset_short_name' :
			case 'asset_name' :
			case 'access_time' :
				echo '<?php echo htmlspecialchars(array_get_index($asset_info,"'.$var.'")); ?>';
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

			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$item	An entry from our contents array
	*
	* @access private
	* @return void
	*/
	function _printContentItemCode($element)
	{
		$area_id = $this->attr('id_name');

		switch (array_get_index($element, 'operation')) {
			case 'asset' :
				echo '
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
				';
				return true;
			default :
				return parent::_printContentItemCode($element);
		}//end switch

	}//end _printContentItemCode()


	/*
	* Return whether this DA has been given sufficient details to be able to print stuff
	*
	* This function can be overriden by children to enforce required attributes etc
	*
	* @return boolean
	* @access protected
	*/
	function _canInit()
	{
		return count($this->attr('asset_types')) && count($this->attr('asset_count'));

	}//end _canInit()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @access protected
	* @return void
	*/
	function _printInitCode()
	{
		$area_id = $this->attr('id_name');
		$exclude_assets = $this->attr('exclude_assets');
		$this_types = $this->attr('asset_types');
		$max_count = $this->attr('asset_count');
		$time_format = $this->attr('time_format');

		echo '
			<?php
				$exclude_assets = '.var_export($exclude_assets,true).';
				$allowed_asset_types = '.var_export($this_types,true).';
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
						$query = true;
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
						// if this url already exists in history, remove it
						$history_index = array_get_index($history_hash, $url);

						if (!is_null($history_index)) {
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
						$new_entry["access_time"]		= date("'.$time_format.'");

						array_unshift($history, $new_entry);

						// update history hash
						foreach ($history as $index => $contents) {
							$new_history_hash[$contents["asset_link"]] = $index;
						}

						$_SESSION["design_areas"]["'.$area_id.'"]["history"] = $history;
						$_SESSION["design_areas"]["'.$area_id.'"]["history_hash"] = $new_history_hash;

						// now that the session is saved, we want to remove the first item because
						// we do not want to list the current page
						array_shift($history);
					}
				}

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
	* @access public
	* @return array(string)
	*/
	function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'asset_contents';
		return $res;

	}//end getProtectedAttrs()


}//end class
?>