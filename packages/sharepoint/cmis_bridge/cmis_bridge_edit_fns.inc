<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: cmis_bridge_edit_fns.inc,v 1.3 2011/04/07 00:56:03 mhaidar Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* CMIS_Bridge_Edit_Fns
*
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class CMIS_Bridge_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for editing WSDL URLs
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	*/
	function paintWsdlUrl(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$wsdl = $asset->attr('wsdl');
		
		?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<?php echo 'Service Name'; ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo 'WSDL URL'; ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo 'Connection Status'; ?>
						</td>
					</tr>
				<?php

				foreach ($wsdl as $service => $url) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $service; ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								if ($write_access){
									text_box($prefix.'[wsdl_urls]['.$service.']', $url, '100');
								} else {
									echo $url;
								}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								$conn = $asset->connect($service, TRUE);
								if ($conn === FALSE) {
									echo '<span style="color: red"><b>'.translate('cmis_not_connected').'</b></span>';
								} else {
									echo '<span style="color: green"><b>'.translate('cmis_connected').'</b></span>';
								}
							?>
						</td>
					</tr>
					<?php
				}//end foreach data
				?>
				</table>
		<?php
		return TRUE;

	}//end paintWsdlUrl()


	/**
	* Process the interface for editing WSDL URLs
	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	 */
	function processWsdlUrl(&$asset, &$o, $prefix)
	{
        if (!$asset->writeAccess('attributes')) return FALSE;

        if (isset($_POST[$prefix]['wsdl_urls'])) {
	        $wsdl = $_POST[$prefix]['wsdl_urls'];
	        foreach ($wsdl as $service => $url){
	        	$wsdl[$service] = trim($url);
	        }
	
			$asset->setAttrValue('wsdl', $wsdl);
			$asset->saveAttributes();
        }
        
		return TRUE;

	}//end processWsdlUrl()


	/**
	* paint function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintSynchCheck(&$asset, &$o, $prefix)
	{
		$wa	= $asset->writeAccess('attributes');
		if ($wa) check_box($prefix.'_synch');
		return TRUE;

	}//end paintSynchCheck()

	
	/**
	* process function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processSynchCheck(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_synch'])) {
			$vars['bridge_id'] = $asset->id;
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_sync_cmis_bridge', $vars, '', SQ_PACKAGES_PATH.'/sharepoint/hipo_jobs/');
		}
		
		return TRUE;
	}//end processSynchCheck()
	
	
	/**
	* Paint the inteface for configuring metadata-fields used for synchronization
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function paintMetadataOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$metadata_fields = $asset->attr('metadata_fields');
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo "CMIS Property"; ?></th>
					<th><?php echo "Metadata Field Type"; ?></th>
					<th><?php echo "Metadata Field ID" ?></th>
				</tr>
			<?php
			foreach ($metadata_fields as $property => $info) {
				?>
				<tr>
					<td>
						<?php
							echo $property;
						?>
					</td>
					<td>
						<?php
							$type = substr(strrchr($info['type_code'], "_"), 1);
							echo ucwords($type);
						?>
					</td>
					<td>
						<?php
						if ($wa) {
							asset_finder($prefix.'_'.$property, $info['field'], Array($info['type_code'] => 'I'));
						} else {
							if ($info['field']) echo get_asset_tag_line($info['field']);
						}
						?>
					</td>
				</tr>
				<?php
			}//end foreach
			?>
			</table>
			<?php
		return TRUE;
		
	}//end paintMetadataOptions()


	/**
	* Process the inteface for configuring metadata-fields used for synchronization
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function processMetadataOptions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$applied_schemas = $mm->getSchemas($asset->id, TRUE, TRUE);
		$fields = $mm->getMetadataFields($applied_schemas);
		
        $metadata_fields = $asset->attr('metadata_fields');	
		
        foreach ($metadata_fields as $property => $info){
        	 $field_id = (isset($_POST[$prefix.'_'.$property]['assetid'])) ? $_POST[$prefix.'_'.$property]['assetid'] : '';
        	 if (!empty($field_id) && !array_key_exists($field_id, $fields)){
        	 	trigger_localised_error('SHAR0001', E_USER_WARNING, $field_id);
        	 	continue;	
        	 }
        	 $metadata_fields[$property]['field'] = $field_id;
        }

		$asset->setAttrValue('metadata_fields', $metadata_fields);
		$asset->saveAttributes();
		
		return TRUE;

	}//end processMetadataOptions()
	
	
	
}//end class
?>
