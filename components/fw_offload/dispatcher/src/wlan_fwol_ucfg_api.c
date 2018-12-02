/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/**
 * DOC: define internal APIs related to the fwol component
 */

#include "wlan_fw_offload_main.h"
#include "wlan_fwol_ucfg_api.h"

QDF_STATUS ucfg_fwol_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = fwol_cfg_on_psoc_enable(psoc);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("Failed to initialize FWOL CFG");

	return status;
}

void ucfg_fwol_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	/* Clear the FWOL CFG Structure */
}

/**
 * fwol_psoc_object_created_notification(): fwol psoc create handler
 * @psoc: psoc which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect psoc is created
 *
 * Return QDF_STATUS status in case of success else return error
 */
static QDF_STATUS
fwol_psoc_object_created_notification(struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = qdf_mem_malloc(sizeof(*fwol_obj));
	if (!fwol_obj)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_FWOL,
						       fwol_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("Failed to attach psoc_ctx with psoc");
		qdf_mem_free(fwol_obj);
	}

	return status;
}

/**
 * fwol_psoc_object_destroyed_notification(): fwol psoc delete handler
 * @psoc: psoc which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect psoc is deleted
 *
 * Return QDF_STATUS status in case of success else return error
 */
static QDF_STATUS fwol_psoc_object_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct wlan_fwol_psoc_obj *fwol_obj;
	QDF_STATUS status;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_FWOL,
						       fwol_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("Failed to detach psoc_ctx from psoc");
		return status;
	}

	qdf_mem_free(fwol_obj);

	return status;
}

QDF_STATUS ucfg_fwol_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("unable to register psoc create handle");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("unable to register psoc create handle");
		wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	}

	return status;
}

void ucfg_fwol_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("unable to unregister psoc destroy handle");

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("unable to unregister psoc create handle");
}

QDF_STATUS
ucfg_fwol_get_coex_config_params(struct wlan_objmgr_psoc *psoc,
				 struct wlan_fwol_coex_config *coex_config)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	*coex_config = fwol_obj->cfg.coex_config;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_fwol_get_thermal_temp(struct wlan_objmgr_psoc *psoc,
			   struct wlan_fwol_thermal_temp *thermal_info)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	*thermal_info = fwol_obj->cfg.thermal_temp_cfg;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_fwol_get_ie_whitelist(struct wlan_objmgr_psoc *psoc, bool *ie_whitelist)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	*ie_whitelist = fwol_obj->cfg.ie_whitelist_cfg.ie_whitelist;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_fwol_set_ie_whitelist(struct wlan_objmgr_psoc *psoc, bool ie_whitelist)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	fwol_obj->cfg.ie_whitelist_cfg.ie_whitelist = ie_whitelist;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_ani_enabled(struct wlan_objmgr_psoc *psoc,
				     bool *ani_enabled)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*ani_enabled = fwol_obj->cfg.ani_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_enable_rts_sifsbursting(struct wlan_objmgr_psoc *psoc,
					    bool *enable_rts_sifsbursting)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_rts_sifsbursting = fwol_obj->cfg.enable_rts_sifsbursting;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_max_mpdus_inampdu(struct wlan_objmgr_psoc *psoc,
				      uint8_t *max_mpdus_inampdu)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*max_mpdus_inampdu = fwol_obj->cfg.max_mpdus_inampdu;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_arp_ac_category(struct wlan_objmgr_psoc *psoc,
				    uint32_t *arp_ac_category)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*arp_ac_category = fwol_obj->cfg.arp_ac_category;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_enable_phy_reg_retention(struct wlan_objmgr_psoc *psoc,
					     uint8_t *enable_phy_reg_retention)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_phy_reg_retention = fwol_obj->cfg.enable_phy_reg_retention;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_fwol_get_all_whitelist_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_fwol_ie_whitelist *whitelist)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	*whitelist = fwol_obj->cfg.ie_whitelist_cfg;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_upper_brssi_thresh(struct wlan_objmgr_psoc *psoc,
				       uint16_t *upper_brssi_thresh)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*upper_brssi_thresh = fwol_obj->cfg.upper_brssi_thresh;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_lower_brssi_thresh(struct wlan_objmgr_psoc *psoc,
				       uint16_t *lower_brssi_thresh)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*lower_brssi_thresh = fwol_obj->cfg.lower_brssi_thresh;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_enable_dtim_1chrx(struct wlan_objmgr_psoc *psoc,
				      bool *enable_dtim_1chrx)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_dtim_1chrx = fwol_obj->cfg.enable_dtim_1chrx;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_get_alternative_chainmask_enabled(struct wlan_objmgr_psoc *psoc,
				       bool *alternative_chainmask_enabled)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get fwol obj");
		return QDF_STATUS_E_FAILURE;
	}

	*alternative_chainmask_enabled =
				fwol_obj->cfg.alternative_chainmask_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_get_smart_chainmask_enabled(struct wlan_objmgr_psoc *psoc,
					    bool *smart_chainmask_enabled)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*smart_chainmask_enabled =
				fwol_obj->cfg.smart_chainmask_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_rts_profile(struct wlan_objmgr_psoc *psoc,
				     uint16_t *get_rts_profile)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*get_rts_profile = fwol_obj->cfg.get_rts_profile;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_enable_fw_log_level(struct wlan_objmgr_psoc *psoc,
					     uint16_t *enable_fw_log_level)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_fw_log_level = fwol_obj->cfg.enable_fw_log_level;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_enable_fw_log_type(struct wlan_objmgr_psoc *psoc,
					    uint16_t *enable_fw_log_type)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_fw_log_type = fwol_obj->cfg.enable_fw_log_type;
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_RA_FILTERING
QDF_STATUS ucfg_fwol_set_is_rate_limit_enabled(struct wlan_objmgr_psoc *psoc,
					       bool is_rate_limit_enabled)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	fwol_obj->cfg.is_rate_limit_enabled = is_rate_limit_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_is_rate_limit_enabled(struct wlan_objmgr_psoc *psoc,
					       bool *is_rate_limit_enabled)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*is_rate_limit_enabled = fwol_obj->cfg.is_rate_limit_enabled;
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_TSF
QDF_STATUS ucfg_fwol_get_tsf_gpio_pin(struct wlan_objmgr_psoc *psoc,
				      uint32_t *tsf_gpio_pin)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*tsf_gpio_pin = fwol_obj->cfg.tsf_gpio_pin;
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS ucfg_fwol_get_tsf_gpio_pin(struct wlan_objmgr_psoc *psoc,
				      uint32_t *tsf_gpio_pin)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef DHCP_SERVER_OFFLOAD
QDF_STATUS
ucfg_fwol_get_enable_dhcp_server_offload(struct wlan_objmgr_psoc *psoc,
					 bool *enable_dhcp_server_offload)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*enable_dhcp_server_offload = fwol_obj->cfg.enable_dhcp_server_offload;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_fwol_get_dhcp_max_num_clients(struct wlan_objmgr_psoc *psoc,
					      uint32_t *dhcp_max_num_clients)
{
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL obj");
		return QDF_STATUS_E_FAILURE;
	}

	*dhcp_max_num_clients = fwol_obj->cfg.dhcp_max_num_clients;
	return QDF_STATUS_SUCCESS;
}
#endif
