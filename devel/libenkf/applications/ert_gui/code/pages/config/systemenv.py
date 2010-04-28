#----------------------------------------------------------------------------------------------
# System tab
# ----------------------------------------------------------------------------------------------
from widgets.pathchooser import PathChooser
from widgets.configpanel import ConfigPanel
from widgets.tablewidgets import KeywordTable, KeywordList
import ertwrapper
from PyQt4 import QtGui, QtCore
from pages.config.jobs.jobspanel import JobsPanel, Job
import os

def createSystemPage(configPanel, parent):
    configPanel.startPage("System")

    r = configPanel.addRow(PathChooser(parent, "Job script", "job_script", True))
    r.initialize = lambda ert : [ert.setTypes("site_config_get_job_script", ertwrapper.c_char_p),
                                 ert.setTypes("site_config_set_job_script", None, ertwrapper.c_char_p)]
    r.getter = lambda ert : ert.enkf.site_config_get_job_script(ert.site_config)
    r.setter = lambda ert, value : ert.enkf.site_config_set_job_script(ert.site_config, str(value))

    internalPanel = ConfigPanel(parent)
    internalPanel.startPage("setenv")

    r = internalPanel.addRow(KeywordTable(parent, "", "setenv"))
    r.initialize = lambda ert : [ert.setTypes("site_config_get_env_hash"),
                                 ert.setTypes("site_config_clear_env", None),
                                 ert.setTypes("site_config_setenv", None, [ertwrapper.c_char_p, ertwrapper.c_char_p])]
    r.getter = lambda ert : ert.getHash(ert.enkf.site_config_get_env_hash(ert.site_config))

    def setenv(ert, value):
        ert.enkf.site_config_clear_env(ert.site_config)
        for env in value:
            ert.enkf.site_config_setenv(ert.site_config, env[0], env[1])

    r.setter = setenv

    internalPanel.endPage()

    internalPanel.startPage("Update path")

    r = internalPanel.addRow(KeywordTable(parent, "", "update_path"))
    r.initialize = lambda ert : [ert.setTypes("site_config_get_path_variables"),
                                 ert.setTypes("site_config_get_path_values"),
                                 ert.setTypes("site_config_clear_pathvar", None),
                                 ert.setTypes("site_config_update_pathvar", None,
                                              [ertwrapper.c_char_p, ertwrapper.c_char_p])]
    def get_update_path(ert):
        paths = ert.getStringList(ert.enkf.site_config_get_path_variables(ert.site_config))
        values =  ert.getStringList(ert.enkf.site_config_get_path_values(ert.site_config))

        return [[p, v] for p, v in zip(paths, values)]

    r.getter = get_update_path

    def update_pathvar(ert, value):
        ert.enkf.site_config_clear_pathvar(ert.site_config)

        for pathvar in value:
            ert.enkf.site_config_update_pathvar(ert.site_config, pathvar[0], pathvar[1])

    r.setter = update_pathvar

    internalPanel.endPage()


    internalPanel.startPage("Jobs")

    r = internalPanel.addRow(JobsPanel(parent))
    r.initialize = lambda ert : [ert.setTypes("site_config_get_installed_jobs"),
                                 ert.setTypes("ext_job_is_private", ertwrapper.c_int, library=ert.job_queue),
                                 ert.setTypes("ext_job_get_config_file", ertwrapper.c_char_p, library=ert.job_queue),
                                 ert.setTypes("ext_job_set_config_file", None, ertwrapper.c_char_p, library=ert.job_queue),
                                 ert.setTypes("ext_job_alloc", argtypes=ertwrapper.c_char_p, library=ert.job_queue, selfpointer=False),
                                 ert.setTypes("ext_job_fscanf_alloc", argtypes=[ertwrapper.c_char_p, ertwrapper.c_char_p, ertwrapper.c_char_p], library=ert.job_queue, selfpointer=False),
                                 ert.setTypes("ext_joblist_get_job", argtypes=ertwrapper.c_char_p, library=ert.job_queue),
                                 ert.setTypes("ext_joblist_del_job", ertwrapper.c_int, ertwrapper.c_char_p, library=ert.job_queue),
                                 ert.setTypes("ext_joblist_has_job", ertwrapper.c_int, ertwrapper.c_char_p, library=ert.job_queue),
                                 ert.setTypes("ext_joblist_add_job", None, [ertwrapper.c_char_p, ertwrapper.c_long], library=ert.job_queue),
                                 ert.setTypes("ext_joblist_get_jobs", library=ert.job_queue)]
    def get_jobs(ert):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        h  = ert.job_queue.ext_joblist_get_jobs(jl)

        jobs = ert.getHash(h, return_type=ertwrapper.c_long)

        private_jobs = []
        for k, v in jobs:
            #print k, v
            v = int(v)
            path = ert.job_queue.ext_job_get_config_file(v)
            job = Job(k, path)
            private_jobs.append(job)
            #print k, ert.job_queue.ext_job_get_config_file(v)
            #if ert.job_queue.ext_job_is_private(v):
            #    private_jobs.append(k)

        return private_jobs

    def update_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)

        if os.path.exists(value.path):
            #license = ert.enkf.site_config_get_license_root_path__(ert.site_config) todo: missing function
            job = ert.job_queue.ext_job_fscanf_alloc(value.name, "/tmp", value.path)
            ert.job_queue.ext_joblist_add_job(jl, value.name, job)
        else:
            job = ert.job_queue.ext_joblist_get_job(jl, value.name)
            ert.job_queue.ext_job_set_config_file(job, value.path)

        for job in get_jobs(ert):
            print job.name, job.path

    def add_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        if not ert.job_queue.ext_joblist_has_job(jl, value.name):
            job = ert.job_queue.ext_job_alloc(value.name)
            ert.job_queue.ext_job_set_config_file(job, value.path)
            ert.job_queue.ext_joblist_add_job(jl, value.name, job)
            return True

        return False

    def remove_job(ert, value):
        jl = ert.enkf.site_config_get_installed_jobs(ert.site_config)
        success = ert.job_queue.ext_joblist_del_job(jl, value.name)

        if not success:
            QtGui.QMessageBox.question(parent, "Failed", "Unable to delete job!", QtGui.QMessageBox.Ok)
            return False
        return True


    r.getter = get_jobs
    r.setter = update_job
    r.insert = add_job
    r.remove = remove_job
    
    internalPanel.endPage()
    configPanel.addRow(internalPanel)

    configPanel.endPage()




