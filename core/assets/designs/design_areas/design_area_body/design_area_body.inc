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
* $Id: design_area_body.inc,v 1.39 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Body
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.39 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Body extends Design_Area
{


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
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		$format = $this->attr('format');

		echo '
		<','?php';
		// NB we are leaving PHP mode here so that
		// the following code is echoed rather than executed
		?>

		$body = '';
		if (SQ_IN_LIMBO) {

			// Print the Simple Edit Interface
			$ASSET->printLimbo();

		} else if ((array_get_index($_GET, 'SQ_ACTION') == 'diff') && ($ASSET->status & SQ_SC_STATUS_SAFE_EDITING) && $ASSET->writeAccess('')) {

			// Show the diff between system version and edited version

			// Get the live contents of the asset being displayed
			ob_start();
				$ASSET->printBodyWithPaintLayout();
				$edited_contents = ob_get_contents();
			ob_end_clean();

			// Set the force-system-version flag
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('force_system_version', TRUE);

				// Reload all assets that were used during the previous printbody,
				// so they know they have to use the system version
				$used_asset_ids = array_unique($GLOBALS['SQ_SYSTEM']->am->_get_asset_history);
				foreach ($used_asset_ids as $used_asset_id) {
					$used_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($used_asset_id);
					if($used_asset)
					    $used_asset->reload();
				}
				$ASSET->reload();

				// Get the safe edit contents of the asset being displayed
				ob_start();
					$ASSET->printBodyWithPaintLayout();
					$system_version_contents = ob_get_contents();
				ob_end_clean();

			// Unset the force-system-version flag
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('force_system_version', FALSE);

			// Reload all assets that were used during the printbody,
			// so they no longer think they have to use the system version
			$used_asset_ids = array_unique($GLOBALS['SQ_SYSTEM']->am->_get_asset_history);
			foreach ($used_asset_ids as $used_asset_id) {
				$used_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($used_asset_id);
				if($used_asset)
				    $used_asset->reload();
			}
			$ASSET->reload();

			// Show the differences
			require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/design_area_body/matrix_diff_highlighter.inc';
			$highlighter = new Matrix_Diff_Highlighter();
			echo $highlighter->process($system_version_contents, $edited_contents);

		} else {

			// Print the body as normal
			if ($ASSET->_is_cacheable) {
				$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$body = $cm->loadFromCache($ASSET->id, $ASSET->type());
			}

			// If cache MISS, or not cacheable at all
			if (!$ASSET->_is_cacheable || ($body === FALSE)) {
				ob_start();
					$ASSET->printBodyWithPaintLayout();
					$body = ob_get_contents();
				ob_end_clean();
			}

			<?php
			switch ($format) {
				case 'low_bandwidth':
					// low bandwidth design - strip images, embeds, objects, and additional whitespace
					echo '
						$body = preg_replace(\'|\s+|\', \' \', $body);
						$body = str_replace(\'<br />\', \'<br/>\', $body);
						$body = str_replace(\'<br/>\', "\n", $body);
						$body = preg_replace(\'|\n\s+|\', "\n", $body);
						$body = preg_replace(\'|[\n]+|\', "\n", $body);
						$body = str_replace("\n", \'<br/>\', $body);
						';

				// break is missing on purpose

				case 'text':
					// text only design - strip images, embeds and objects
					echo '
						$body = preg_replace(\'|\<img([^\>]*)\>|Ui\',\'\',$body);
						$body = preg_replace(\'|\\<embed(.*)\>(.*)</embed>|i\',\'\',$body);
						$body = preg_replace(\'|\\<object(.*)\>(.*)</object>|i\',\'\',$body);
						';
				break;
			}
			?>
			echo $body;

		}//end else print normally
		<?php
		echo '
		?','>';

		return TRUE;

	}//end _printPaintingCode()


	/**
	* Returns true if this content generated by this design area can be cached
	*
	* @return boolean
	* @access public
	*/
	public function canBeCached()
	{
		return FALSE;

	}//end canBeCached()


}//end class
?>
