<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Bodycopy extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy($assetid=0)
	{
		$this->Asset($assetid);
	}


	/**
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return false;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// save a version of the bodycopy as it appears now
		$edit = $this->getEditFns();
		ob_start();
		$edit->paint($this);
		$output = ob_get_contents();
		ob_end_clean();

		$e = '/\\.\\/\\?a=([0-9]+)/';
		$matches = Array();
		preg_match_all($e, $output, $matches);
		$replace_assetids = $matches[1];

		// replace all ./?a=xx links with php echo's
		$e = "/?\./\\?a=([0-9]+)([^0-9]+)";
		$with = '<?php echo $sq_asset_url_list[\\1]; ?>';
		$output = ereg_replace($e, $with.'\\2', $output);

		// save the contents of the bodycopy
		if (!string_to_file($output, $this->data_path.'/.sq_system/content_file.php')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create content file', E_USER_WARNING);
			return false;
		}

		// save the links we currently use (./?a=xx)
		if (!string_to_file(serialize($replace_assetids), $this->data_path.'/.sq_system/.notice_links')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create notice links file', E_USER_WARNING);
			return false;
		}

		return true;
	}//end saveSystemVersion()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a Bodycopy with a blank name', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create an table
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy_table');
		$bodycopy_table =& new Bodycopy_Table();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'dependant' => 1);
		$bodycopy_table->setAttrValue('name', 'Table 1');
		if (!$bodycopy_table->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy_table')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_TYPE_2 => Array('bodycopy_table' => Array('card' => 'M', 'exclusive' => false))
					);

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function paintBackend(&$backend)
	{
		switch($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				require_once(dirname(__FILE__).'/popups/'.trim($_REQUEST['popup_file']));
				exit();
			default :
				return parent::paintBackend($backend);

		}// end switch

	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		if ($this->useSystemVersion()) {
			if (file_exists($this->data_path.'/content_file.php')) {
				// create an array of urls that the content file is
				// going to try and use to work out file urls, link hrefs etc
				$sq_asset_url_list = Array();
				
				$notice_links = unserialize(file_to_string($this->data_path.'/.notice_links'));

				foreach ($notice_links as $assetid) {
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetHref($assetid);
					if ($href == '') $href = "Javascript: alert('This link does not exist');";
					$sq_asset_url_list[$assetid] = $href;
				}

				bam($sq_asset_url_list);

				include($this->data_path.'/content_file.php');
			}
		} else {
			$am = &$GLOBALS['SQ_SYSTEM']->am;

			// get all the tables linked to this bodycopy
			$tables = $this->getLinks(SQ_LINK_TYPE_2, 'bodycopy_table');

			// paint each table
			foreach ($tables as $table_data) {
				$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
				$table->printBody($table);
			}
		}
	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	}//end _getName();


}//end class
?>
