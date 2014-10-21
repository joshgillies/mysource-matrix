<?php
	require_once dirname(__FILE__).'/../../../../core/include/init.inc';

	require_once SQ_SYSTEM_ROOT.'/core/lib/html_form/html_form.inc';
	if (isset($GLOBALS['SQ_SYSTEM']->user)) {
		$user = $GLOBALS['SQ_SYSTEM']->user;
	} else {
		$user = NULL;
	}//end if

	// Check for at least simple edit access
	if (is_null($user) || $user instanceof Public_User) {
	    echo 'You are not allowed to access this page';
	    exit();
	}

?>

	<?php
		require_once SQ_SYSTEM_ROOT.'/core/include/backend_outputter.inc';
		require_once SQ_SYSTEM_ROOT.'/core/include/asset_edit_interface.inc';

		$o = new Backend_Outputter();

		$o->openSection('Keyword List for Simple Edit Keywords');
			$type_list = Array();
			$asset_type_list = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();
			foreach ($asset_type_list as $key => $value) {
				if ($value['instantiable'] == 1) {
					$type_list[] = $key;
				}
			}

			$asset_types = Array();

			foreach ($type_list as $asset_type) {
				$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type);
				$asset_types[$asset_type] = $type_info['name'];
			}
			asort($asset_types);

			$o->openField('', 'wide_col');
				echo '<p style="margin: 0;">Select your asset type: ';
				if (isset($_REQUEST['asset_type']))
				{
					combo_box('asset_type', $asset_types, FALSE, $_REQUEST['asset_type']);
				} else {
					combo_box('asset_type', $asset_types);
				}
				echo '&nbsp;';
				submit_button('limbo_keywords_submit', 'Get Keywords');
				echo '</p>';
			$o->closeField();
		$o->closeSection();

		if (isset($_REQUEST['asset_type']))
		{
			$asset_type = preg_replace('/[^a-zA-Z0-9_]+/', '', $_REQUEST['asset_type']);
			$ei= new Asset_Edit_Interface($asset_type);
			$ei->getSimpleEditKeywords($asset_type, $o);
		}
			$o->openField('', 'commit');
				normal_button('cancel', translate('close_window'), 'window.close()', '');
			$o->closeField();
		$o->closeSection();
		$o->paint();
		?>

	</body>
</html>
