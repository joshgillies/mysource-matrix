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
* +--------------------------------------------------------------------+s
*
* $Id: bodycopy_edit_fns.inc,v 1.73 2013/10/02 03:01:16 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';


/**
* Bodycopy_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.73 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['tagging']);

	}//end constructor


	/**
	* Save the ids of all css files attached to this url to be used by the wysiwyg editor later on
	*
	* @param Backend_Outputter	$o	the Backend_Outputter class
	* @param Boolean  $mute_errors 	mutes error reported , default is false hence will report all the errors
	*
	* @return void
	* @access private
	*/
	protected function _saveCssIds(Backend_Outputter $o, $mute_errors = FALSE)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if (isset($GLOBALS['sq_preview_url']) && !empty($GLOBALS['sq_preview_url'])) {
			$url = $GLOBALS['sq_preview_url'];
			$url = preg_replace('|^http[s]?://|', '', $url);

			$designid = $am->getValueFromURL($url, 'design::%frontend%', TRUE);

			if ($designid) {
				$design = $am->getAsset($designid);
				$stylesheets = $design->getStylesheets();

				// save the ids of all the css files into a global var so the wysiwyg style
				// drop down plugin can access this information without another querey
				// we can also limit the stylesheets that are available to it here if we want
				$GLOBALS['sq_bodycopy_cssids'] = array_keys($stylesheets);

				$am->forgetAsset($design);
				unset($design);

			} else {
				if (!$mute_errors) {
					$o->note(translate('bodycopy_asset_has_no_design', $GLOBALS['sq_preview_url']));
				}
			}
		} else {
			if (!$mute_errors) {
				$o->note(translate('bodycopy_asset_has_no_preview'));
			}
		}

	}//end _saveCssIds()


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param Bodycopy			$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintBodycopy(Bodycopy $asset, Backend_Outputter $o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('content');
		// variable to determine whether or not to allow commit
		$print_commit_button = $write_access;

		// save css ids into global var
		$this->_saveCssIds($o);

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		if (!$write_access) {
			ob_start();
			foreach ($containers as $container_data) {
				$container = $am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
				if ($container->useSystemVersion()) {
					$keyword_replacements = Array(); //Do not want to replace keywords
					$container->printBody($keyword_replacements);
				} else {
					$container_edit = $container->getEditFns();
					$container_edit->paintContainer($container, $o, $prefix);
				}
			}
			$html = ob_get_clean();
			$html = preg_replace('|<script\b[^>]*>(.*?)</script>|is', '', $html);
			echo $html;
			return FALSE;
		}

		// the initial width and height to have the layer display created at;
		$init_layer_width  = 500;
		$init_layer_height = 500;

		// all available content types
		$content_types = Array();
		require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';
		$types = Content_Type::getAvailableContentTypes();
		foreach ($types as $type => $type_data) {
			$content_types[$type] = Array('name' => str_replace(' Content Type', '', $type_data['name']));
		}

		?>
		<input type="hidden" name="bodycopy_action" value="">
		<input type="hidden" name="bodycopy_name"   value="">
		<input type="hidden" name="bodycopy_data"   value="">

		<script type="text/javascript">

			// php vars
			var backendHref = "<?php echo $asset->getBackendHref('show_popup');?>";
			var init_layer_width = <?php echo $init_layer_width?>;
			var init_layer_height = <?php echo $init_layer_height?>;
			var bodycopy_types = '<?php echo var_serialise($content_types, TRUE)?>';
			var _prefix = "<?php echo $prefix; ?>";
			var asset_id = "<?php echo $asset->id; ?>";
		</script>

		<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/bodycopy/js/bodycopy_edit_fns.js'?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/layer_handler.js';?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js';?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/debug.js';?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js';?>"></script>


		<?php
		// print JS functions for each of the container types we have installed
		$container_types = $am->getTypeDescendants('bodycopy_container');
		foreach ($container_types as $type) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$type_asset = new $type();
			$edit_fns = $type_asset->getEditFns();
			$edit_fns->paintGenericBackend($asset, $o, $prefix);
		}

		$container_type_options = Array();
		$default = '';
		foreach ($container_types as $type) {
			$code = strtolower(substr($type,9));
			$container_type_options[$code] = ucwords(str_replace('_', ' ', substr($type,9)));
			if (empty($default)) $default = $code;
		}
		asort($container_type_options);
		echo hidden_field($prefix.'_insert_container_type', $default);
		?>

		<!-- Begin Popup -->
		<div id="bodycopyPopupDiv" class="sq-bodycopy-popup-div" style="position: absolute; z-index:-1; visibility: hidden; width: <?php echo $init_layer_width?>; height: <?php echo $init_layer_height?>;">
			<iframe name="bodycopyFrame" id="bodycopyFrame" scrolling="no" width="<?php echo $init_layer_width?>" height="<?php echo $init_layer_height?>" frameborder="no"></iframe>
		</div>
		<!-- End Popup -->

		<?php
			//The following CSS should only get printed if we are not in _admin mode. They are only here to support old versions of EES.
		if (!(SQ_IN_BACKEND || SQ_IN_LIMBO)) {
		?>
			<style>
			.sq-edit-bodycopy-table table{
				width: 100%;
			}
			.sq-edit-bodycopy-table .sq-edit-bodycopy-section{
				margin-bottom: 20px;
			}
			.sq-edit-bodycopy-insert-container-table td{
				font-size: 12px;
			}
			.sq-edit-bodycopy-insert-container-table tr td:first-child{
				width: 20px;
				padding-left: 5px;
			}
			.bodyCopyIcons img {
				margin-right: 5px;
			}
			.sq-content_type_wysiwyg-wrapper.wysiwygArea{
				padding-bottom: 0 !important;
			}
			#ees_editContents .htmlarea-toolbar,
			.htmlarea-toolbar{
				margin: 0 !important;
			}
			.htmlarea-toolbar table{
				margin: 0;
			}
			.htmlarea-toolbar td{
				border: 1px solid #ccc !important;
			}
			#ToolBox{
				border: 1px solid #777;
				color: #000;
				background-color: #FFF;
				position: absolute;
				z-index: 1000;
				text-align: left;
				font-style: normal;
				font-variant: normal;
				font-weight: normal;
				font-size: 100%;
				line-height: normal;
				font-family: Tahoma, Arial;
				visibility: hidden;
			}
			#ToolBoxTitle{
				padding: 2px;
				text-align: left;
				font-weight: bold;
				font-style: normal;
				font-variant: normal;
				font-size: 110%;
				line-height: normal;
				font-family: helvetica, arial, sans-serif;
				background-image: url(/__data/ees/Images/popupTitle-bg.gif);
				background-attachment: scroll;
				background-color: #113262;
				color: #FFF;
				background-position: 0px 0px;
				background-repeat: repeat no-repeat;
			}
			#ToolBoxContent{
				padding: 2px;
				font-style: normal;
				font-variant: normal;
				font-weight: normal;
				font-size: 100%;
				line-height: normal;
				font-family: Tahoma, Arial;
				background-color: #FFF;
			}
			.sq-toolbox-properties-table td{
				padding: 2px;
			}
			</style>
		<?php
		}
		?>

		<table class="sq-edit-bodycopy-table bodytext">

		<?php

		// require library functions needed for drawing the bodycopy
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');

		// for each container, print it's editing interface
		$i = 0;
		foreach ($containers as $container_data) {
			$container = $am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			$container_edit = $container->getEditFns();

			if ($write_access) {
				?>
				<tr>
					<td>
						<input type="hidden" name="bodycopy_saved[<?php echo $prefix?>][<?php echo $container->id?>]" value="">
						<table class="sq-edit-bodycopy-section sq-edit-bodycopy-insert-container-table">
							<tr>
								<?php
								if ( $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DIV_DISABLE_CREATE') === 'yes'){
									?>
									<td colspan="2"><span class="sq-backend-warning"><?php echo translate('bodycopy_pref_cannot_insert_new'); ?></span></td>
									<?php
								}
								else {
									?>
									<td>
										<?php
											Bodycopy_Edit_Fns::printBodycopyIcon("document.main_form.{$prefix}_insert_container_type.value = document.main_form.{$prefix}_insert_container_type_before_{$i}.value; bodycopy_insert_container('$prefix', $i, true)", translate('insert_container'), translate('select_container_type'), 'create_container', '['.translate('insert').']');
										?>
									</td>
									<td class="sq-backend-data" valign="top">
										<?php
										echo '&nbsp;&nbsp;'.translate('create_new').' ';
										echo combo_box($prefix.'_insert_container_type_before_'.$i, $container_type_options, FALSE, '');
										?>
									</td>
								<?php
								}
								?>
							</tr>
						</table>
					</td>
				</tr>
				<?php
			}//end if write access

			?>
			<tr>
			<td class="sq-bodycopy-container-cell bodycopyContainerCell" data-bodycopyid="<?php echo htmlentities($asset->id) ?>" data-containerid="<?php echo htmlentities($container->id) ?>"<?php
				if ((boolean) $container->writeAccess('content') === FALSE) {
					echo ' data-readonly="true"';
				}
			?>><?php

			if ($container_edit->paintContainer($container, $o, $prefix) == TRUE) {
				$print_commit_button = TRUE;
			}
			?></td>
			</tr>
			<?php

			$i++;
		}//end foreach container

		if ($write_access) {
			?>
			<tr>
				<td>
					<table class="sq-edit-bodycopy-insert-container-table">
						<tr>
							<?php
							if ( $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DIV_DISABLE_CREATE') === 'yes'){
								?>
								<td colspan="2"><span class="sq-backend-warning"><?php echo translate('bodycopy_pref_cannot_insert_new'); ?></span></td>
								<?php
							}
							else {
								?>
								<td>
									<?php
									Bodycopy_Edit_Fns::printBodycopyIcon("document.main_form.{$prefix}_insert_container_type.value = document.main_form.{$prefix}_insert_container_type_after_{$i}.value; bodycopy_insert_container('$prefix', $i, false)", translate('insert_container'), translate('select_container_type'), 'create_container', '['.translate('insert').']');
									?>
								</td>
								<td class="sq-backend-data" valign="top">
									<?php
									echo '&nbsp;&nbsp;'.translate('create_new').' ';
									echo combo_box($prefix.'_insert_container_type_after_'.$i, $container_type_options, FALSE, '');
									?>
								</td>
							<?php
							}
							?>
						</tr>
					</table>
				</td>
			</tr>
			<?php
		}//end if write access

		?>
		</table>

		<?php

		return $print_commit_button;

	}//end paintBodycopy()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param Bodycopy			$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processBodycopy(Bodycopy $asset, Backend_Outputter $o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$updated = FALSE;

		// save css ids into global var
		// we dont want note to be displayed so we mute errors
		$this->_saveCssIds($o,TRUE);

		// do some thing for the first time we process a bodycopy on this page
		if (empty($this->_tmp['update_initialised'])) {
			$bc_action = (empty($_POST['bodycopy_action'])) ? ''      : $_POST['bodycopy_action'];
			$bc_name   = (empty($_POST['bodycopy_name']))   ? ''      : $_POST['bodycopy_name'];
			$bc_data   = (empty($_POST['bodycopy_data']))   ? Array() : $_POST['bodycopy_data'];
			$bc_saved  = (empty($_POST['bodycopy_saved']))  ? Array() : $_POST['bodycopy_saved'];

			if (empty($bc_data) && empty($bc_saved)) {
				return FALSE;
			}

			// if there is some bodycopy data, deserialise it into the arrays
			// so that everything else can use it
			if ($bc_data) $bc_data = var_unserialise($bc_data);

			// unserialize the data for a bodycopy.
			for (reset($bc_saved); NULL !== ($k = key($bc_saved)); next($bc_saved)) {
				$containers =& $bc_saved[$k];
				for (reset($containers); NULL !== ($containersid = key($containers)); next($containers)) {
					$containers[$containersid] = var_unserialise($containers[$containersid]);
				}
			}

			unset($containers);

			$this->_tmp['update_initialised'] = TRUE;
		}

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);

		// process each of the containers
		foreach ($containers as $container_data) {
			$container = $am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			$container_edit = $container->getEditFns();
			if ($container_edit->processContainer($container, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)) {
				if ($container->saveAttributes()) $updated = TRUE;
			}
		}

		// only update if we are the bodycopy that submitted the form
		if ($bc_name == $prefix || isset($bc_saved[$prefix])) {
			// perform some bodycopy level actions
			if ($bc_name == $prefix) {
				switch ($bc_action) {
					case 'insert_container' :
						if ($this->insertContainer($asset, $bc_data['container_type'], $bc_data['containerid'], $bc_data)) {
							$updated = TRUE;
						}
					break;

					case 'delete_container' :
						if ($this->deleteContainer($asset, $bc_data['containerid'])) {
							$asset->_updated();
							$updated = TRUE;
						}
					break;
				}//end switch
			}
		}

		return $updated;

	}//end processBodycopy()


	/**
	* Insert a new container into this bodycopy
	*
	* @param Bodycopy	$asset			the bodycopy
	* @param string		$type			the type of the container (table, div etc)
	* @param int		$containerid	the ID of the container to insert before/after
	* @param array		$bc_data		the array of bodycopy data that got posted
	*
	* @return boolean
	* @access public
	*/
	public function insertContainer(Bodycopy $asset, $type, $containerid, Array $bc_data)
	{
		// Save the current setup of links, if it has not been saved already
		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			if (!file_exists($asset->data_path.'/.safe_edit_before')) {
				$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
				file_put_contents($asset->data_path.'/.safe_edit_before', serialize($containers));
			}//end if
		}//end if

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		$num_containers = count($containers);

		$containerid = intval($containerid);

		// if the containerid is greater than the number of containers we have, set it to the last one
		if ($containerid > $num_containers - 1) {
			$containerid = $num_containers - 1;
		} else if ($containerid < 0) {
			$divid = 0;
		}

		// if they want to insert after the passed containerid what
		// they really mean is to insert before the next containerid
		if (!$bc_data['before']) $containerid++;

		// now add the container
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => $containerid, 'is_dependant' => 1);

		$asset_type = 'bodycopy_'.$type;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
		$new_container = new $asset_type();
		// Bug #5460 New DIVs created in Safe Edit showing on front end before going Live. See Bodycopy_Container::saveSystemVersion().
		$new_container->_tmp['skip_save_system_version_content_file'] = TRUE;

		$new_container->setAttrValue('name', 'New '.ucfirst($type));
		$new_container->setAttrValue('attributes', $bc_data['attributes']);
		$retval = (bool) $new_container->create($copy_link, $bc_data);
		if ($retval && (!isset($bc_data['attributes']['identifier']) || trim($bc_data['attributes']['identifier']) == '')) {
			// try and update the name to be unique - but dont stress if we couldn't update it
			$new_container->setAttrValue('name', 'New '.ucfirst($type).' '.$new_container->id);
			$new_container->saveAttributes();
		}

		// Tweaking this asset is under safe edit and has a workflow on it
		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$schemas = $wfm->getSchemas($asset->id, TRUE);
			if (!empty($schemas) && $new_container->status != SQ_STATUS_EDITING) {
				// Drop to a run level to force the status change to SQ_STATUS_EDITING (saveSystemVersion() so it can go back to LIVE)
				$GLOBALS['SQ_SYSTEM']->setRunLevel(191);
				$new_container->saveSystemVersion();
				$new_container->processStatusChange(SQ_STATUS_EDITING, FALSE);
				$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($new_container->id);
				if (!empty($children)) {
					foreach (array_keys($children) as $child_assetid) {
						$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_assetid, '', TRUE);
						if (!is_null($child)) {
							$child->saveSystemVersion();
							$child->processStatusChange(SQ_STATUS_EDITING, FALSE);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($child);
						}//end if
					}//end foreach
				}//end if
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}//end if
		}//end if

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_container);

		// Save the current setup of links
		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
			file_put_contents($asset->data_path.'/.safe_edit_after', serialize($containers));
		}//end if

		return $retval;

	}//end insertContainer()


	/**
	* Unlink a container from this bodycopy (effectivly deleting it from this bodycopy)
	*
	* @param Bodycopy	$asset			the bodycopy
	* @param int		$containerid	the ID of the container to delete
	*
	* @return boolean
	* @access public
	*/
	public function deleteContainer(Bodycopy $asset, $containerid)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		foreach ($containers as $container_data) {
			if ($container_data['minorid'] == $containerid) {
				// found the container to delete
				return $asset->deleteLink($container_data['linkid']);
			}
		}

		return FALSE;

	}//end deleteContainer()


	/**
	* Print an icon for a bodycopy action
	*
	* @param string	$onclick	JS to execute when the icon is clicked
	* @param string	$heading	The heading for the tooltip
	* @param string	$desc		The main body of the tooltip
	* @param string	$icon		The filename base of the icon to display (must be a PNG)
	* @param string	$alt		Alternate text for the icon
	* @param string	$extra		Any extra HTML you want to put in the IMG tag
	* @param string	$width		The width of the icon
	* @param string	$height		The height of the icon
	*
	* @return void
	* @access public
	*/
	public static function printBodycopyIcon($onclick, $heading, $desc, $icon, $alt='', $extra='', $width='16', $height='16')
	{
		$path = sq_web_path('data').'/asset_types/bodycopy/images/icons/'.$icon.'.png';
		$extra .= ' onmouseover="if (bodycopy_initialised) tooltip.show(this, \''.addslashes(htmlentities($desc, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'\', \''.addslashes(htmlentities($heading, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'\')"';
		$extra .= ' onmouseout="if (bodycopy_initialised) tooltip.hide()"';
		$extra .= ' onclick="'.$onclick.'"';
		$extra .= ' align="absmiddle" ';
		if (!empty($onclick)) {
			$extra .= ' style="cursor:pointer;"';
		}
		sq_print_icon($path, $width, $height, $alt, '', $extra);

	}//end printBodycopyIcon()


}//end class


?>
