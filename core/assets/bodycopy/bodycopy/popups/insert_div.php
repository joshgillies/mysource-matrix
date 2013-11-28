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
* $Id: insert_div.php,v 1.15 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Insert DIV Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]  = owner.form_element_value(f.identifier);
		data["layout_type"] = owner.form_element_value(f.layout_type);
		data["css_class"]   = owner.form_element_value(f.css_class);
		data["content_type"] = owner.form_element_value(f.content_type);
		owner.bodycopy_save_insert_div(data);
	}

</script>
<?php
	require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';
	$content_types = Content_Type::getAvailableContentTypes();
	$default_content_type = $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DEFAULT_CONTENT_TYPE');
	$default_pres_type = $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DEFAULT_PRESENTATION_TYPE');
	$possible_types = Array(
					'div'	=> translate('block-level'),
					'span'	=> translate('inline'),
					'none'	=> translate('raw_html'),
				  );
?>
<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('insert_div'); ?>
</h1>
<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table>
	<tr>
		<td colspan="2">
		<fieldset>
			<legend><?php echo translate('identification'); ?></legend>
			<table>
				<tr>
					<td class="label"><?php echo translate('name'); ?>:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset>
			<legend><?php echo translate('style_information'); ?></legend>
			<table>
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('presentation'); ?>:</td>
					<td>
						<?php combo_box('layout_type', $possible_types, FALSE, $default_pres_type); ?>
					</td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('class'); ?>:</td>
					<td><input type="text" name="css_class" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset class="last">
			<legend><?php echo translate('content_type'); ?></legend>
			<table>
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('content_type'); ?>:</td>
					<td>
						<select name="content_type" id="content_type">
							<?php
								foreach ($content_types as $type_code => $data) {
									$selected_text = '';
									if ($type_code == $default_content_type) {
										$selected_text = 'SELECTED';
									}
									?>
										<option value="<?php echo $type_code; ?>" <?php echo $selected_text; ?>><?php echo str_replace(' Content Type', '', $data['name']); ?></option>
									<?php
								}
							?>
						</select>
					</td>
				</tr>
			</table>
		</fieldset>
	</tr>
	<tr class="sq-popup-footer">
		<td align="left">
			<input type="button" class="" name="cancel" onClick="javascript: popup_close();" value="<?php echo translate('cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('insert'); ?>"/>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>