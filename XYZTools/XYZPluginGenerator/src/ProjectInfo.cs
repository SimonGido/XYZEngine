using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace XYZPluginGenerator
{
    internal class ProjectInfo
    {
        public string Language;

        public string ProjectKind;

        public ProjectInfo(object project, Type type)
        {
            ExtractFields(project, type);
        }

        public List<string> Validate()
        {
            List<string> errors = new List<string>();
            Type thisType = GetType();
            foreach (var thisField in thisType.GetFields())
            {
                if (thisField.GetValue(this) == null)
                {
                    errors.Add("Missing value " + thisField.Name);
                }
            }
            return errors;
        }

        private void ExtractFields(object project, Type type)
        {
            Type thisType = GetType();
            foreach (var thisField in thisType.GetFields())
            {
                foreach (var field in type.GetFields())
                {
                    TryExtract(thisField, project, field);
                }
            }
        }

        private void TryExtract(FieldInfo dstInfo, object srcIntance, FieldInfo srcInfo)
        {
            if (dstInfo.Name == srcInfo.Name && dstInfo.FieldType.Name == srcInfo.FieldType.Name)
                dstInfo.SetValue(this, srcInfo.GetValue(srcIntance));
        }
    }
}
